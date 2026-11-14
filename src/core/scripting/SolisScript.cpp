#include "SolisScript.hpp"

#include "core/logger/LogHelper.hpp"

// The Solis compiler is plain C
extern "C" {
    #include "Solis/compiler/check.h"
    #include "Solis/compiler/common.h"
    #include "Solis/compiler/emit.h"
    #include "Solis/compiler/gen.h"
    #include "Solis/compiler/module.h"
}

#include <cstdlib>
#include <cstring>

namespace {
    void LogDiagnostics(const loaded* modules, const diag_vec& diags) {
        for (int i = 0; i < diags.len; i++) {
            const diag& d = diags.at[i];
            const module* m = d.module ? module_find(modules, d.module) : nullptr;

            if (!m) {
                LOGF(WARNING, "{}", d.message);
                continue;
            }

            const line_col pos = line_col_of(m->src, m->src_len, d.at.start);
            LOGF(WARNING, "{}:{}:{}: {}", m->path, pos.line, pos.col, d.message);
        }
    }

    void CollectFunctions(const loaded* modules, std::vector<std::string>& functions) {
        const module* root = module_find(modules, modules->root);
        if (!root) {
            return;
        }

        for (int i = 0; i < root->prog->items.len; i++) {
            const item& entry = root->prog->items.at[i];
            if (entry.kind != IT_FUNC) {
                continue;
            }

            const func* fn = entry.as.fn;
            if (!fn->body || fn->generics.len > 0 || fn->params.len > 0) {
                continue;
            }

            if (std::strcmp(fn->name, "main") != 0) {
                functions.emplace_back(fn->name);
            }
        }
    }
}

bool SolisScript::Compile(const std::string& path, const std::string& host, CompiledScript& script) {
    script = {};
    return GetInstance().CompileImpl(path, host, script.image, &script.functions);
}

bool SolisScript::CompileImpl(const std::string& path, const std::string& host, std::vector<uint8_t>& image, std::vector<std::string>* functions) {
    arena arena_state;
    arena_init(&arena_state);

    sl_interner* interner = intern_new(&arena_state);

    diag_vec diags{};
    // "" disables the prelude file; scripts get the host API from `import host;`
    loaded* modules = module_load(&arena_state, interner, path.c_str(), "", host.empty() ? nullptr : host.c_str(), &diags);

    uint8_t* bytes = nullptr;
    size_t length = 0;

    if (diags.len == 0) {
        decls* declarations = check_modules(&arena_state, interner, modules, &diags);
        if (diags.len == 0) {
            bc_program* program = generate(&arena_state, interner, modules, declarations, &diags);
            if (program && diags.len == 0) {
                bytes = bc_serialize(program, &length);
            }
        }
    }

    if (diags.len == 0 && bytes && length > 0) {
        image.assign(bytes, bytes + length);

        if (functions) {
            CollectFunctions(modules, *functions);
        }
    } else if (diags.len == 0) {
        LOGF(WARNING, "Solis produced no bytecode for {}", path);
    } else {
        LogDiagnostics(modules, diags);
    }

    std::free(bytes);
    arena_free(&arena_state);

    return !image.empty();
}
