/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kernel.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:29:00 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 19:36:48 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/kernel.h"

multiboot_info_t *g_mbi = 0;

void kernel_set_multiboot_info(multiboot_info_t *mbi)
{
	g_mbi = mbi;
}

void page_fault_handler(uint32_t error_code)
{
	uint32_t faulting_addr;
	faulting_addr = read_cr2();

	// decode error code bits
	int present		= error_code & 0x1;	// 0 = not present, 1 = protection violation
	int rw			= error_code & 0x2;	// 0 = read, 1 = write
	int user		= error_code & 0x4;	// 0 = kernel, 1 = user mode
	int reserved	= error_code & 0x8;	// reserved bit overwrite

	printk("ERROR", "addr=0x%x error=0x%x\n", faulting_addr, error_code);

	if (!present)
		printk("ERROR", "cause: page not present\n");
	if (rw)
		printk("ERROR", "cause: write to read-only page\n");
	if (user)
		printk("ERROR", "cause: user mode access\n");
	if (reserved)
		printk("ERROR", "cause: reserved bits overwritten\n");

	if (user)
		kwarn("page fault in user space — would kill process");
	else
		kpanic("page fault in kernel space");
}

void init(uint32_t magic, multiboot_info_t *mbi)
{
	/* Set up the GDT, which is required for paging and other protected-mode features. */
	gdt_init();
	/* Initialize terminal interface */
	terminal_initialize();

	if (magic != MULTIBOOT_MAGIC)
		kpanic("Invalid multiboot magic number");

	kernel_set_multiboot_info(mbi);

	/* Initialize the physical frame allocator and virtual memory manager. */
	pfa_init(mbi);

	// mark pd and pt frames as used in the PFA
	pfa_mark_used((uintptr_t)pd, sizeof(pd));
	pfa_mark_used((uintptr_t)pt, sizeof(pt));

	/* Set up paging (virtual memory). */
	paging_init();

	/* Set up the IDT and PIC.*/
	idt_init();
	pic_init();

}

void kernel_main(uint32_t magic, multiboot_info_t *mbi)
{
	init(magic, mbi);

	/* Register interrupt handlers for keyboard and page faults. */
	idt_set_gate(33, (uintptr_t)keyboard_handler_stub, 0x08, 0x8E);
	idt_set_gate(14, (uintptr_t)page_fault_handler_stub, 0x08, 0x8E);
	/* Enable interrupts now that everything is set up. */
	__asm__ volatile ("sti");

	terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
	/* Main loop: wait for keyboard input and print it to the terminal. */
	for (;;) {}
}
