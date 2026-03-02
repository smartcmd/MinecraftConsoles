; iob_shim.asm
; Provides __iob_func for legacy libs compiled against pre-VS2015 CRT

.code
EXTRN __acrt_iob_func:PROC

__iob_func PROC
    mov     ecx, 0
    jmp     __acrt_iob_func
__iob_func ENDP

END