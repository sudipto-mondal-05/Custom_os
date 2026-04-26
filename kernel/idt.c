#include "idt.h"

idt_entry_t idt[256];
idt_ptr_t   idt_ptr;

void idt_set_gate(unsigned char num, unsigned int base, unsigned short selector, unsigned char flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector  = selector;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}

void idt_init() {
    idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_ptr.base  = (unsigned int) &idt;

    // Zero out entire IDT
    unsigned char* ptr = (unsigned char*) &idt;
    for (int i = 0; i < sizeof(idt_entry_t) * 256; i++)
        ptr[i] = 0;

    // Load IDT directly with inline assembly
    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}
