#include "kernel.h"

struct idt_entry idt[IDT_SIZE];

void dump_kernel_stack(size_t words)
{
	uintptr_t	esp;
	uintptr_t	ebp;
	uint32_t	*ptr;
	size_t		i;

	__asm__ volatile ("mov %%esp, %0" : "=r"(esp));
	__asm__ volatile ("mov %%ebp, %0" : "=r"(ebp));

	printk("STACK", "esp=0x%x ebp=0x%x words=%u\n",
		(unsigned int)esp, (unsigned int)ebp, (unsigned int)words);

	ptr = (uint32_t *)esp;

	i = 0;
	while (i < words)
	{
		terminal_setcolor(vga_entry_color(i + 1, VGA_COLOR_BLACK));
		printk("STACK", "[+0x%x] @0x%x = 0x%x\n",
			(unsigned int)(i * sizeof(uint32_t)),
			(unsigned int)(uintptr_t)(ptr + i),
			ptr[i]);
		i++;
	}
}

void idt_set_gate(uint8_t num, uint32_t handler_address,
				  uint16_t selector, uint8_t type_attr)
{
	idt[num].offset_low  = handler_address & 0xFFFF;
	idt[num].offset_high = (handler_address >> 16) & 0xFFFF;
	idt[num].selector    = selector;
	idt[num].zero        = 0;
	idt[num].type_attr   = type_attr;
}

void idt_init(void)
{
	struct idt_ptr ptr;
	ptr.limit = (sizeof(struct idt_entry) * IDT_SIZE) - 1;
	ptr.base  = (uintptr_t)&idt;
	load_idt(&ptr);
}

void pic_init(void)
{
	// ICW1: start initialization sequence
	outb(0x20, 0x11);  // master
	io_wait();
	outb(0xA0, 0x11);  // slave
	io_wait();

	// ICW2: remap offsets
	outb(0x21, 0x20);  // master IRQs start at interrupt 32 (0x20)
	io_wait();
	outb(0xA1, 0x28);  // slave IRQs start at interrupt 40 (0x28)
	io_wait();

	// ICW3: tell master/slave how they're connected
	outb(0x21, 0x04);  // master: slave is on IRQ2
	io_wait();
	outb(0xA1, 0x02);  // slave: its cascade identity
	io_wait();

	// ICW4: set 8086 mode
	outb(0x21, 0x01);
	io_wait();
	outb(0xA1, 0x01);
	io_wait();

	// Mask all IRQs except IRQ1 (keyboard)
	outb(0x21, 0xFD);  // 11111101 - only IRQ1 unmasked
	outb(0xA1, 0xFF);  // all slave IRQs masked
}

void kernel_main(void)
{
	/* Initialize terminal interface */
	terminal_initialize();

	terminal_setrow(VGA_HEIGHT / 2);
	terminal_setcolumn((VGA_WIDTH - 2) / 2);

	terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_LIGHT_BLUE));
	terminal_writestring("4");
	terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_LIGHT_RED));
	terminal_writestring("2\n");
	terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
	terminal_setcolumn((VGA_WIDTH - 23) / 2);
	terminal_writestring("Hello\t kernel\t\tWorld!\n\n");

	/* Set up the IDT and PIC, then enable interrupts. */
	idt_init();
	pic_init();
	idt_set_gate(33, (uintptr_t)keyboard_handler_stub, 0x08, 0x8E);
	__asm__ volatile ("sti");

	terminal_setrow(0);
	terminal_setcolumn(0);
	terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
	dump_kernel_stack(8);
	terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
	printk("INFO", "kprintf/printk ready\n");
	kprintf("VGA %ux%u | test: %d %u 0x%x %c %s\n",
		VGA_WIDTH, VGA_HEIGHT, -42, 42U, 42U, 'A', "ok");
	terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

	/* Main loop: wait for keyboard input and print it to the terminal. */
	while (1)
		__asm__ volatile ("hlt");
}
