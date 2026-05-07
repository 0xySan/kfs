/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   idt.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:28:55 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:28:55 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/kernel.h"

struct idt_entry idt[IDT_SIZE];

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