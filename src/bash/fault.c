/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fault.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:50:58 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:50:59 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

int fault(void)
{
	printk("BASH", "This is a test of the page fault handler. It will attempt to read from an invalid address, which should trigger a page fault and invoke the handler.\n");
	volatile int *ptr = (int *)0xDEADBEEF; // Invalid address
	int value = *ptr; // This should cause a page fault
	(void)value; // Suppress unused variable warning
	return 0;
}