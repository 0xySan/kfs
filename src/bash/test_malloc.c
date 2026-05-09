/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_malloc.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:28:41 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/09 21:46:33 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

void kshow_free_space(void)
{
	block_header_t	*current;
	size_t			free_bytes;
	size_t			used_bytes;
	size_t			free_blocks;
	size_t			used_blocks;

	if (!heap_start)
	{
		printk("HEAP", "heap not initialized\n");
		return;
	}

	free_bytes = used_bytes = free_blocks = used_blocks = 0;
	current = heap_start;
	while (current)
	{
		if (current->free)
		{
			free_bytes += current->size;
			free_blocks++;
		}
		else
		{
			used_bytes += current->size;
			used_blocks++;
		}
		current = current->next;
	}

	printk("HEAP", "free:  %u bytes (%u blocks)\n", free_bytes, free_blocks);
	printk("HEAP", "used:  %u bytes (%u blocks)\n", used_bytes, used_blocks);
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