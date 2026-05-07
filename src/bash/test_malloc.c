/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_malloc.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:28:41 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:37:10 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

void kshow_free_space(void)
{
	if (!heap_start)
	{
		printk("HEAP", "heap not initialized\n");
		return;
	}

	size_t free_bytes = 0;
	size_t used_bytes = 0;

	block_header_t *current = heap_start;
	while (current)
	{
		if (current->free)
			free_bytes += current->size;
		else
			used_bytes += current->size;
		current = current->next;
	}

	printk("HEAP", "free:  %u bytes\n", free_bytes);
	printk("HEAP", "used:  %u bytes\n", used_bytes);
	printk("HEAP", "total: %u bytes\n", free_bytes + used_bytes);
}

int test_malloc(char *argv[])
{
	if (argv[1])
	{
		size_t words = (size_t)atoi(argv[1]);
		if (words > 0)
		{
			char *a = kmalloc(words);
			printk("BASH", "kmalloc(%u) returned 0x%x\n", (unsigned int)words, (uintptr_t)a);
			if (a)
			{
				for (size_t i = 0; i < words; i++)
					a[i] = (char)(i % 256);
				for (size_t i = 0; i < words; i++)
				{
					if (a[i] != (char)(i % 256))
					{
						printk("BASH", "Memory corruption detected at index %u: expected 0x%x, got 0x%x\n",
							(unsigned int)i, (unsigned int)(char)(i % 256), (unsigned int)a[i]);
						return 1;
					}
				}
			}
			if (argv[2] && (ft_strcmp(argv[2], "true") == 0
						|| ft_strcmp(argv[2], "1") == 0
						|| ft_strcmp(argv[2], "free") == 0))
			{
				kfree(a);
				printk("BASH", "Memory freed\n");
			}
		}
		else
			printk("BASH", "Usage: malloc [size]\n");
	}
	else
		printk("BASH", "Usage: malloc [size] [free {true/false}]\n");
	return 0;
}