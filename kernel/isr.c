#define VIDEO_MEMORY 0xB8000
#define RED_ON_BLACK 0x04

extern void isr0();  extern void isr1();  extern void isr2();
extern void isr3();  extern void isr4();  extern void isr5();
extern void isr6();  extern void isr7();  extern void isr8();
extern void isr13(); extern void isr14();
extern void irq1();

void timer_init();
void pic_init();
void idt_set_gate(unsigned char num, unsigned int base, unsigned short selector, unsigned char flags);

const char* exception_messages[] = {
    "EXCEPTION: Division By Zero",
    "EXCEPTION: Debug",
    "EXCEPTION: Non Maskable Interrupt",
    "EXCEPTION: Breakpoint",
    "EXCEPTION: Overflow",
    "EXCEPTION: Bound Range Exceeded",
    "EXCEPTION: Invalid Opcode",
    "EXCEPTION: Device Not Available",
    "EXCEPTION: Double Fault",
    "EXCEPTION: Reserved",
    "EXCEPTION: Reserved",
    "EXCEPTION: Reserved",
    "EXCEPTION: Reserved",
    "EXCEPTION: General Protection Fault",
    "EXCEPTION: Page Fault"
};

void print_error(const char* msg) {
    unsigned char* vid = (unsigned char*) VIDEO_MEMORY;
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        vid[i]   = ' ';
        vid[i+1] = RED_ON_BLACK;
    }
    for (int i = 0; msg[i] != 0; i++) {
        vid[i*2]   = msg[i];
        vid[i*2+1] = RED_ON_BLACK;
    }
}

void isr_handler(unsigned int int_no) {
    if (int_no < 15)
        print_error(exception_messages[int_no]);
    else
        print_error("EXCEPTION: Unknown");

    __asm__ volatile("cli; hlt");
}

void isr_init() {
    idt_set_gate(0,  (unsigned int)isr0,  0x10, 0x8E);
    idt_set_gate(1,  (unsigned int)isr1,  0x10, 0x8E);
    idt_set_gate(2,  (unsigned int)isr2,  0x10, 0x8E);
    idt_set_gate(3,  (unsigned int)isr3,  0x10, 0x8E);
    idt_set_gate(4,  (unsigned int)isr4,  0x10, 0x8E);
    idt_set_gate(5,  (unsigned int)isr5,  0x10, 0x8E);
    idt_set_gate(6,  (unsigned int)isr6,  0x10, 0x8E);
    idt_set_gate(7,  (unsigned int)isr7,  0x10, 0x8E);
    idt_set_gate(8,  (unsigned int)isr8,  0x10, 0x8E);
    idt_set_gate(13, (unsigned int)isr13, 0x10, 0x8E);
    idt_set_gate(14, (unsigned int)isr14, 0x10, 0x8E);
    idt_set_gate(33, (unsigned int)irq1,  0x10, 0x8E);

    timer_init();
}
