/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inline.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:29:43 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:29:43 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INLINE_H
# define INLINE_H

#include "define.h"

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static inline void outb(uint16_t port, uint8_t value)
{
	__asm__ volatile ("outb %0, %1"
		:
		: "a"(value), "Nd"(port)
	);
}

static inline void outw(uint16_t port, uint16_t value)
{
	__asm__ volatile ("outw %0, %1"
		:
		: "a"(value), "Nd"(port)
	);
}

static inline uint8_t inb(uint16_t port)
{
	uint8_t value;
	__asm__ volatile ("inb %1, %0"
		: "=a"(value)
		: "Nd"(port)
	);
	return value;
}

static inline void io_wait(void)
{
	outb(0x80, 0);
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

#endif // INLINE_H
