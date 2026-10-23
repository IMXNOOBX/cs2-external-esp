#include "SolisScript.hpp"

#include "core/logger/LogHelper.hpp"

// The Solis compiler is plain C.
extern "C" {
    #include "Solis/compiler/check.h"
    #include "Solis/compiler/common.h"
    #include "Solis/compiler/emit.h"
    #include "Solis/compiler/gen.h"
    #include "Solis/compiler/module.h"
}

// Solis uses fopen, so disable the warning
#pragma warning(push)
#pragma warning(disable: 4996)
#include "solis.hpp"
#pragma warning(pop)

#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <vector>

namespace scripting {
    bool Compile(const std::string& path, std::vector<uint8_t>& image) {
        arena arena_state;
        arena_init(&arena_state);

        sl_interner* interner = intern_new(&arena_state);

        diag_vec diags{};
        loaded* modules = module_load(&arena_state, interner, path.c_str(), nullptr, &diags);

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
        }

        std::free(bytes);
        arena_free(&arena_state);

        return !image.empty();
    }

    bool RunSolisScript(const std::string& path) {
        if (!std::filesystem::exists(path)) {
            LOGF(WARNING, "404 Solis script not found at: {}", path);
            return false;
        }

        std::vector<uint8_t> image;
        std::string error;
        if (!Compile(path, image)) {
            LOGF(WARNING, "Failed to compile {}", path);
            return false;
        }


        LOGF(WARNING, "Runing script {}", path);

        try {
            solis::Vm vm;
            vm.openStd();

            vm.define("print", [](solis::Vm&, solis::Args args) {
                for (int i = 0; i < args.size(); i++) {
                    if (i) {
                        std::cout << ' ';
                    }
                    std::cout << args[i].toString();
                }
                std::cout << '\n';
                return solis::Value();
            });

            vm.load(image);
            vm.run();
            return true;
        }
        catch (const solis::Error& e) {
            LOGF(WARNING, "Solis error in {}: {}", path, e.what());
            return false;
        }
    }
}
