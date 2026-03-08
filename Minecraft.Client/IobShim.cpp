// Original file is iob_shim.asm
// Rewritten in C because the build system breaks on linux-windows crosscomp with this
// rm: __declspec(naked) is seemingly(?) not working on x86_64 (atleast via wine); use __attribute__((naked)) instead.

#if 0
.code
EXTRN __acrt_iob_func:PROC

__iob_func PROC
    mov     ecx, 0
    jmp     __acrt_iob_func
__iob_func ENDP

END
#endif

#include <cstdio>

extern "C" FILE *__cdecl __acrt_iob_func(unsigned index);

#if defined(_MSC_VER) && defined(__clang__)
extern "C" [[__gnu__::__naked__]] FILE *__cdecl __iob_func(void)
{
    __asm__(
        ".intel_syntax noprefix\n\t"
        "mov ecx, 0\n\t"
        "jmp __acrt_iob_func\n\t"
        ".att_syntax prefix");
}
#endif
