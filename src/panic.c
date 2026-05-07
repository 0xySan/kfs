/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   panic.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:29:11 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:29:11 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/kernel.h"

void kpanic(const char *msg)
{
	__asm__ volatile ("cli"); // disable interrupts to prevent further issues
	printk("PANIC", "%s\n", msg);
	kernel_halt_forever();
}

void kwarn(const char *msg)
{
	printk("WARN", "%s\n", msg);
}