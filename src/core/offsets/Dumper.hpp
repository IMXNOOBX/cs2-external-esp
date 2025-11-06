#pragma once
#include "core/memory/Memory.hpp"
#include "Offsets.hpp"

inline const DWORD MAX_BLOCK_SIZE = 409600;

class Dumper {
public:
    ~Dumper()                           = default;
    Dumper(const Dumper&)            = delete;
    Dumper(Dumper&&)                 = delete;
    Dumper& operator=(const Dumper&) = delete;
    Dumper& operator=(Dumper&&)      = delete;

   static bool Init();
private:
    Dumper() {};

    static Dumper& GetInstance()
    {
        static Dumper i{};
        return i;
    }

    bool InitImpl();
private:
    std::vector<WORD> StrSigToArray(const std::string& sig);
    DWORD64 Scan(const std::string sig, ProcessModule module);
    void GetNextArray(std::vector<short>& next, const std::vector<WORD>& signature);
    std::vector<DWORD64> ScanMemory(const std::string& sig, DWORD64 start, DWORD64 end, int number = 1);
    void ScanBlock(byte* buffer, const std::vector<short>& next, const std::vector<WORD>& signature, DWORD64 start, DWORD size, std::vector<DWORD64>& result);
};