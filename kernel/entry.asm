[BITS 32]
global _start
extern kernel_main

section .multiboot
align 8
    dd 0xE85250D6
    dd 0
    dd 16
    dd -(0xE85250D6 + 0 + 16)
    dw 0
    dw 0
    dd 8

section .text
_start:
    mov esp, 0x90000
    call kernel_main
    jmp $
