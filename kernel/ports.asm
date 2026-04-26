[BITS 32]
global outb
global inb
global pic_init

outb:
    mov dx, [esp+4]    ; port
    mov al, [esp+8]    ; value
    out dx, al
    ret

inb:
    mov dx, [esp+4]    ; port
    in al, dx
    ret

pic_init:
    ; PIC1 init
    mov al, 0x11
    out 0x20, al
    out 0xA0, al

    ; Vector offsets
    mov al, 0x20
    out 0x21, al       ; PIC1 → IRQ 32
    mov al, 0x28
    out 0xA1, al       ; PIC2 → IRQ 40

    ; Cascading
    mov al, 0x04
    out 0x21, al
    mov al, 0x02
    out 0xA1, al

    ; 8086 mode
    mov al, 0x01
    out 0x21, al
    out 0xA1, al

    ; Unmask keyboard IRQ1 only, mask everything else
    mov al, 0xFD       ; 11111101 - only IRQ1 unmasked
    out 0x21, al
    mov al, 0xFF       ; mask all PIC2
    out 0xA1, al

    ret
