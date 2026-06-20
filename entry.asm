[bits 32]
global _start
extern kernel_main

_start:
    call kernel_main  ; Safely invoke your C++ code function loop
    cli               ; Fallback safety lock
    hlt
    jmp _start
