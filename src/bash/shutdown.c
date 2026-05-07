/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shutdown.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:28:31 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:28:31 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

void kernel_shutdown(void)
{
	__asm__ volatile ("cli");

	/* QEMU/Bochs ACPI poweroff ports */
	outw(0x604, 0x2000);
	outw(0xB004, 0x2000);
	outb(0xF4, 0x00);

	kernel_halt_forever();
}

int shutdown(void)
{
	printk("SHUTDOWN", "Shutting down...\n");
	kernel_shutdown();
	return 0;
}
