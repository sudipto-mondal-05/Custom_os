[BITS 32]
global isr0, isr1, isr2, isr3, isr4, isr5, isr6
global isr7, isr8, isr13, isr14
global irq0, irq1

extern isr_handler
extern keyboard_handler
extern timer_handler

%macro ISR_NOERRCODE 1
isr%1:
    cli
    push dword %1
    call isr_handler
    add esp, 4
    iret
%endmacro

%macro ISR_ERRCODE 1
isr%1:
    cli
    add esp, 4
    push dword %1
    call isr_handler
    add esp, 4
    iret
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_ERRCODE   13
ISR_ERRCODE   14

irq0:
    cli
    call timer_handler
    iret

irq1:
    cli
    call keyboard_handler
    iret
