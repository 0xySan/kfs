/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   reboot.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:28:28 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:28:28 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

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

int reboot(void)
{
	printk("REBOOT", "Rebooting the machine...\n");
	kernel_reboot();
	return 0;
}