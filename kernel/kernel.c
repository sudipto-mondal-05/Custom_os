void idt_init();
void isr_init();
void pic_init();
void shell_init();

void kernel_main() {
    pic_init();
    idt_init();
    isr_init();
    shell_init();

    __asm__ volatile("sti");

    while(1) {}
}
