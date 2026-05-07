/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   halt.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:27:49 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:54:44 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

void kernel_halt_forever(void)
{
	for (;;)
		__asm__ volatile ("hlt");
}

int halt(void)
{
	printk("HALT", "Halting forever and ever...\n");
	kernel_halt_forever();
	return 0;
}