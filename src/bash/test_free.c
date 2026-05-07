/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_free.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:28:35 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:28:35 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

void show_free_frames()
{
	uint32_t free_count = 0;
	for (uint32_t i = 0; i < total_frames; i++)
		if (!(bitmap[i / 32] & (1 << (i % 32))))
			free_count++;

	printk("PFA", "free frames=%u (%u MB)\n", free_count, free_count * 4 / 1024);
}

int test_free(char *argv[])
{
	if (!argv[1])
		printk("BASH", "Usage: free [virtual|frames]\n");
	else if (ft_strcmp(argv[1], "virtual") == 0)
		kshow_free_space();
	else if (ft_strcmp(argv[1], "frames") == 0)
		show_free_frames();
	else
		printk("BASH", "Usage: free [virtual|frames]\n");
	return 0;
}