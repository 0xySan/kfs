/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kernel.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:29:00 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 22:06:35 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/kernel.h"
multiboot_info_t *g_mbi = 0;

void kernel_set_multiboot_info(multiboot_info_t *mbi)
{
	g_mbi = mbi;
}

void init(uint32_t magic, multiboot_info_t *mbi)
{
	/* Set up the GDT, which is required for paging and other protected-mode features. */
	gdt_init();
	/* Initialize terminal interface */
	terminal_initialize();

	if (magic != MULTIBOOT_MAGIC)
		kpanic("Invalid multiboot magic number", NULL);

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

	// Register handlers for page faults and keyboard interrupts.
	register_exception_handlers();

	/* Enable interrupts now that everything is set up. */
	__asm__ volatile ("sti");

	terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
	/* Main loop: wait for keyboard input and print it to the terminal. */
	for (;;) {}
}
