void outb(unsigned short port, unsigned char val);

static unsigned int ticks = 0;

void timer_handler() {
    ticks++;
    // Send EOI
    outb(0x20, 0x20);
}

unsigned int timer_get_ticks() {
    return ticks;
}

unsigned int timer_get_seconds() {
    return ticks / 18;  // PIT fires ~18 times per second by default
}

void timer_init() {
    extern void irq0();
    // IRQ0 = IDT entry 32
    extern void idt_set_gate(unsigned char, unsigned int, unsigned short, unsigned char);
    idt_set_gate(32, (unsigned int)irq0, 0x10, 0x8E);
}
