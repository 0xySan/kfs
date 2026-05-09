/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fault.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:50:58 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/09 05:05:18 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

int fault(int argc, char **argv)
{
	if (argc < 2)
	{
		printk("BASH", "Usage: fault <handler_number>\n");
		printk("BASH", "  0 - Divide by zero (INT 0)\n");
		printk("BASH", "  1 - Debug (INT 1)\n");
		printk("BASH", "  3 - Breakpoint (INT 3)\n");
		printk("BASH", "  4 - Overflow (INT 4)\n");
		printk("BASH", "  6 - Invalid opcode (INT 6)\n");
		printk("BASH", "  13 - General protection fault (INT 13)\n");
		printk("BASH", "  14 - Page fault (INT 14)\n");
		printk("BASH", "  128 - Syscall (INT 128)\n");
		return (1);
	}
	int handler = atoi(argv[1]);
	if (handler == 0)
	{
		printk("BASH", "Testing INT 0 - Divide by zero\n");
		int a = 1, b = 0;
		int c = a / b;
		(void)c;
	}
	else if (handler == 1)
	{
		printk("BASH", "Testing INT 1 - Debug\n");
		__asm__ volatile ("int $1");
	}
	else if (handler == 3)
	{
		printk("BASH", "Testing INT 3 - Breakpoint\n");
		__asm__ volatile ("int $3");
	}
	else if (handler == 4)
	{
		printk("BASH", "Testing INT 4 - Overflow\n");
		__asm__ volatile ("int $4");
	}
	else if (handler == 6)
	{
		printk("BASH", "Testing INT 6 - Invalid opcode\n");
		__asm__ volatile (".byte 0x0F, 0x0B");
	}
	else if (handler == 13)
	{
		printk("BASH", "Testing INT 13 - General protection fault\n");
		__asm__ volatile ("mov $0x41, %ax; mov %ax, %ds");
	}
	else if (handler == 14)
	{
		printk("BASH", "Testing INT 14 - Page fault\n");
		volatile int *p = (int *)0xdeadbeef;
		int x = *p;
		(void)x;
	}
	else if (handler == 128)
	{
		printk("BASH", "Testing INT 128 - Syscall\n");
		__asm__ volatile (
			"mov $1, %%eax\n"        // syscall number 1 (sys_exit)
			"mov $0x1234, %%ebx\n"   // arg1 (4660 in decimal)
			"mov $0x5678, %%ecx\n"   // arg2 (22136 in decimal)
			"mov $0x9abc, %%edx\n"   // arg3 (39612 in decimal)
			"mov $0xdef0, %%esi\n"   // arg4 (57328 in decimal)
			"mov $0x1357, %%edi\n"   // arg5 (4951 in decimal)
			"int $0x80"
			::: "eax", "ebx", "ecx", "edx", "esi", "edi"
		);
	}
	else
		printk("BASH", "Invalid handler number\n");
	return (0);
}