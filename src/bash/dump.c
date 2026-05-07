/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dump.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:27:37 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:27:37 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

const char *colors[] = {
	BLACK, BLUE, GREEN,
	CYAN, RED, MAGENTA,
	YELLOW, WHITE
};

const char *colors_bg[] = {
	BLACK_BG, BLUE_BG,
	GREEN_BG, CYAN_BG, RED_BG,
	MAGENTA_BG, YELLOW_BG, WHITE_BG
};

int dump(char *argv[])
{
	if (argv[1])
	{
		size_t words = (size_t)atoi(argv[1]);
		if (words > 0)
			dump_kernel_stack(words);
		else
			printk("BASH", "Usage: dump [nb]\n");
	}
	else
		dump_kernel_stack(8);
	return 0;
}

void dump_kernel_stack(size_t words)
{
	uintptr_t	esp;
	uintptr_t	ebp;
	uint32_t	*ptr;
	size_t		i;

	__asm__ volatile ("mov %%esp, %0" : "=r"(esp));
	__asm__ volatile ("mov %%ebp, %0" : "=r"(ebp));

	printk("STACK", "esp=0x%x ebp=0x%x words=%u\n",
		(unsigned int)esp, (unsigned int)ebp, (unsigned int)words);

	ptr = (uint32_t *)esp;

	i = 0;
	while (i < words)
	{
		printk("STACK", "%s%s[+0x%x] @0x%x = 0x%x%s\n",
			colors_bg[(i / 8) % (sizeof(colors_bg) / sizeof(colors_bg[0]))],
			colors[i % (sizeof(colors) / sizeof(colors[0]))],
			(unsigned int)(i * sizeof(uint32_t)),
			(unsigned int)(uintptr_t)(ptr + i),
			ptr[i],
			RESET);
		i++;
	}
}