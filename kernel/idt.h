#ifndef IDT_H
#define IDT_H

typedef struct {
    unsigned short base_low;
    unsigned short selector;
    unsigned char  always0;
    unsigned char  flags;
    unsigned short base_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed)) idt_ptr_t;

void idt_init();
void idt_set_gate(unsigned char num, unsigned int base, unsigned short selector, unsigned char flags);

#endif
