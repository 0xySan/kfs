#include "../includes/kernel.h"

struct idt_entry idt[IDT_SIZE];
static multiboot_info_t *g_mbi = 0;

void kernel_set_multiboot_info(multiboot_info_t *mbi)
{
	g_mbi = mbi;
}

void kernel_print_multiboot_flags(void)
{
	int i;

	if (!g_mbi)
	{
		printk("KERNEL", "multiboot info not available\n");
		return;
	}
	i = 0;
	while (i <= 12)
	{
		int is_set = (g_mbi->flags >> i) & 1;
		printk("KERNEL", "flag[%d] = ", i);
		if (is_set)
		{
			terminal_setcolor(VGA_COLOR_GREEN);
			kprintf("1\n");
		}
		else
		{
			terminal_setcolor(VGA_COLOR_RED);
			kprintf("0\n");
		}
		terminal_setcolor(VGA_COLOR_LIGHT_GREY);
		i++;
	}
}

void kernel_halt_forever(void)
{
	for (;;)
		__asm__ volatile ("hlt");
}

void kernel_shutdown(void)
{
	__asm__ volatile ("cli");

	/* QEMU/Bochs ACPI poweroff ports */
	outw(0x604, 0x2000);
	outw(0xB004, 0x2000);
	outb(0xF4, 0x00);

	kernel_halt_forever();
}

void kernel_reboot(void)
{
	__asm__ volatile ("cli");

	/* Wait until keyboard controller input buffer is empty */
	while (inb(0x64) & 0x02)
		;

	/* Pulse CPU reset line */
	outb(0x64, 0xFE);

	kernel_halt_forever();
}

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
	terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
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
	io_wait();
	outb(0xA1, 0xFF);  // all slave IRQs masked
	io_wait();
}

void kernel_main(uint32_t magic, multiboot_info_t *mbi)
{
	/* Initialize terminal interface */
	gdt_init();
	terminal_initialize();

	if (magic != MULTIBOOT_MAGIC)
	{
		terminal_set_execute_on_newline(false);
		printk("KERNEL", "Invalid magic number: 0x%x\n", magic);
		kprintf(TERMINAL_PROMPT_TEXT);
		terminal_set_execute_on_newline(true);
		kernel_halt_forever();
	}

	kernel_set_multiboot_info(mbi);

	pfa_init(mbi);

	show_free_frames();
	void *a = pfa_alloc_frame();
	void *b = pfa_alloc_frame();
	void *c = pfa_alloc_frame();
	printk("PFA", "alloc: 0x%x 0x%x 0x%x\n", a, b, c);
	pfa_free_frame(b);
	void *d = pfa_alloc_frame();
	printk("PFA", "after free b, alloc d: 0x%x (should == b)\n", d);
	show_free_frames();

	/* Set up the IDT and PIC, then enable interrupts. */
	idt_init();
	pic_init();
	idt_set_gate(33, (uintptr_t)keyboard_handler_stub, 0x08, 0x8E);
	__asm__ volatile ("sti");

	terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
	/* Main loop: wait for keyboard input and print it to the terminal. */
	for (;;) {}
}
