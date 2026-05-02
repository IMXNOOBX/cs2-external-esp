#include <stdint.h>

extern "C" {
    uintptr_t __security_cookie = 0xBB40E64E5438AD32;

    void __stdcall __security_check_cookie(uintptr_t cookie) {}
    void __cdecl __report_rangecheckfailure() {}

    int _fltused = 0;

    void* __GSHandlerCheck = nullptr;

    void* __guard_check_icall_fptr = nullptr;
    void* __guard_dispatch_icall_fptr = nullptr;
    void* __guard_xfg_check_icall_fptr = nullptr;
    void* __guard_xfg_dispatch_icall_fptr = nullptr;
    void* __guard_xfg_table_dispatch_icall_fptr = nullptr;
    void* __castguard_check_failure_os_handled_fptr = nullptr;
    void* __guard_dispatch_icall_dummy = nullptr;

    struct {
        unsigned int Size;
        unsigned char Padding[256];
    } _load_config_used = { sizeof(_load_config_used), {0} };
}
