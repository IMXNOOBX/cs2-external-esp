#pragma once

#include <cstdint>
#include <string>
#include <vector>

class SolisScript {
public:
    struct CompiledScript {
        std::vector<uint8_t> image;
        std::vector<std::string> functions;
    };

    ~SolisScript() = default;
    SolisScript(const SolisScript&) = delete;
    SolisScript(SolisScript&&) = delete;
    SolisScript& operator=(const SolisScript&) = delete;
    SolisScript& operator=(SolisScript&&) = delete;

    static bool Compile(const std::string& path, CompiledScript& script);
    static bool CompileFunction(const std::string& path, const std::string& function, std::vector<uint8_t>& image);

private:
    SolisScript() {};

    static SolisScript& GetInstance()
    {
        static SolisScript i{};
        return i;
    }

    bool CompileImpl(const std::string& path, std::vector<uint8_t>& image, std::vector<std::string>* functions);
    bool CompileFunctionImpl(const std::string& path, const std::string& function, std::vector<uint8_t>& image);
};
