/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   idt.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:28:55 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 22:00:46 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/kernel.h"

struct idt_entry idt[IDT_SIZE];
extern void *isr_stub_table[256];

void (*handlers[256])(registers_t *) = {0};

void register_isr_handler(uint8_t int_num, void (*handler)(registers_t *))
{
	handlers[int_num] = handler;
}

void generic_isr_handler(registers_t *regs)
{
	uint32_t int_num = regs->int_num;

	// Call the registered handler for this interrupt, if it exists
	if (handlers[int_num])
		handlers[int_num](regs);
	else
		printk("INT", "Unhandled interrupt: %u\n", int_num);

	// Send End of Interrupt (EOI) signal to PICs if this is an IRQ
	if (int_num >= 32)
	{
		if (int_num >= 40)
			outb(0xA0, 0x20);	// slave EOI
		outb(0x20, 0x20);		// master EOI
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
	for (int i = 0; i < IDT_SIZE; i++)
		idt_set_gate(i, (uintptr_t)isr_stub_table[i], 0x08, 0x8E);
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