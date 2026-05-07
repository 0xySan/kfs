/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   paging.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:27:55 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:27:56 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

int paging(void)
{
	uint32_t cr0 = read_cr0();
	printk("PAGING", "CR0 = 0x%x\n", cr0);

	if (cr0 & (1 << 31))
		printk("PAGING", "Paging ENABLED\n");
	else
		printk("PAGING", "Paging DISABLED\n");
	return 0;
}