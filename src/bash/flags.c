/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   flags.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:27:45 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:27:45 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

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

int flags(void)
{
    kernel_print_multiboot_flags();
    return 0;
}