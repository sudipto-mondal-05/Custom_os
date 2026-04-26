all: myos.iso

kernel/entry.o: kernel/entry.asm
	nasm -f elf32 kernel/entry.asm -o kernel/entry.o

kernel/isr.o: kernel/isr.asm
	nasm -f elf32 kernel/isr.asm -o kernel/isr.o

kernel/ports.o: kernel/ports.asm
	nasm -f elf32 kernel/ports.asm -o kernel/ports.o

kernel/kernel.o: kernel/kernel.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -c kernel/kernel.c -o kernel/kernel.o

kernel/idt.o: kernel/idt.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -c kernel/idt.c -o kernel/idt.o

kernel/isr_c.o: kernel/isr.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -c kernel/isr.c -o kernel/isr_c.o

kernel/keyboard.o: kernel/keyboard.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -c kernel/keyboard.c -o kernel/keyboard.o

kernel/shell.o: kernel/shell.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -c kernel/shell.c -o kernel/shell.o

iso/boot/kernel.bin: kernel/entry.o kernel/isr.o kernel/ports.o kernel/kernel.o kernel/idt.o kernel/isr_c.o kernel/keyboard.o kernel/shell.o
	ld -m elf_i386 -T linker.ld -o iso/boot/kernel.bin kernel/entry.o kernel/isr.o kernel/ports.o kernel/kernel.o kernel/idt.o kernel/isr_c.o kernel/keyboard.o kernel/shell.o

myos.iso: iso/boot/kernel.bin
	grub-mkrescue -o myos.iso iso

run: myos.iso
	qemu-system-i386 -cdrom myos.iso -display gtk -device isa-debug-exit

clean:
	rm -f kernel/*.o iso/boot/kernel.bin myos.iso
