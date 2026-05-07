/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   panic.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:29:11 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 21:56:30 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/kernel.h"

void kpanic(const char *msg, registers_t *regs)
{
    printk("PANIC", "%s\n", msg);
	if (regs)
	{
		printk("PANIC", "eax=0x%x ebx=0x%x ecx=0x%x edx=0x%x\n",
			regs->eax, regs->ebx, regs->ecx, regs->edx);
		printk("PANIC", "eip=0x%x cs=0x%x eflags=0x%x\n",
			regs->eip, regs->cs, regs->eflags);
	}
	__asm__ volatile ("cli"); // disable interrupts
    __asm__ volatile ("hlt"); // halt the CPU
}

void kwarn(const char *msg)
{
	printk("WARN", "%s\n", msg);
}