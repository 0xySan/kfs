/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pfa.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:25:14 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 21:56:53 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "memory.h"

uint32_t	*bitmap;		// pointer to where bitmap lives in RAM
uint32_t	bitmap_size;	// number of uint32_t entries
uint32_t	mem_start;		// first managed frame address
uint32_t	total_frames;	// total frames being tracked

// PFA (Physical Frame Allocator) - manages physical RAM, hands out 4KB frames

static void pfa_clear_bit(uint32_t frame_index)
{
	bitmap[frame_index / 32] &= ~(1 << (frame_index % 32));
}

static void pfa_set_bit(uint32_t frame_index)
{
	bitmap[frame_index / 32] |= (1 << (frame_index % 32));
}

void *pmalloc(void)
{
	for (uint32_t i = 0; i < bitmap_size; i++)
	{
		if (bitmap[i] == 0xFFFFFFFF)
			continue;
		for (uint32_t j = 0; j < 32; j++)
		{
			if (!(bitmap[i] & (1 << j)))
			{
				pfa_set_bit(i * 32 + j);
				return (void *)((uintptr_t)(mem_start + (i * 32 + j) * PAGE_SIZE));
			}
		}
	}
	kpanic("pfa_alloc_frame: out of physical memory", NULL);
	return NULL;
}

void pfree(void *frame)
{
	if ((uintptr_t)frame < mem_start) return;
	if ((uintptr_t)frame % PAGE_SIZE != 0) return;

	uint32_t frame_index = ((uintptr_t)frame - mem_start) / PAGE_SIZE;
	pfa_clear_bit(frame_index);
}

void pfa_init(multiboot_info_t *mbi)
{
	if (!(mbi->flags & (1 << 6)))
		kpanic("Memory map not provided by multiboot", NULL);

	multiboot_mmap_entry_t *entry = (multiboot_mmap_entry_t *)(uintptr_t)mbi->mmap_addr;
	while ((uintptr_t)entry < mbi->mmap_addr + mbi->mmap_length)
	{
		if (entry->type == 1 && entry->addr >= 0x100000)
		{
			mem_start = entry->addr;
			total_frames = entry->len / PAGE_SIZE;
			break;
		}
		entry = (multiboot_mmap_entry_t *) ((uintptr_t)entry + entry->size + sizeof(entry->size));
	}

	bitmap_size = (total_frames + 31) / 32; // round up to nearest uint32_t
	bitmap = (uint32_t *) ALIGN_UP((uintptr_t)&kernel_end); // place bitmap after kernel in memory

	for (uint32_t i = 0; i < bitmap_size; i++)
		bitmap[i] = 0xFFFFFFFF;

	uint32_t i = 0;
	while (i < total_frames)
	{
		pfa_clear_bit(i);
		i++;
	}

	uint32_t kernel_start_frame = (0x100000 - mem_start) / PAGE_SIZE;
	uint32_t kernel_end_frame   = (ALIGN_UP((uintptr_t)&kernel_end) - mem_start) / PAGE_SIZE;

	for (uint32_t i = kernel_start_frame; i < kernel_end_frame; i++)
		pfa_set_bit(i);

	uint32_t bitmap_start_frame = ((uintptr_t)bitmap - mem_start) / PAGE_SIZE;
	uint32_t bitmap_end_frame   = (ALIGN_UP((uintptr_t)bitmap + bitmap_size * sizeof(uint32_t)) - mem_start) / PAGE_SIZE;

	for (uint32_t i = bitmap_start_frame; i < bitmap_end_frame; i++)
		pfa_set_bit(i);
}

void pfa_mark_used(uintptr_t start, size_t size)
{
	if (start < mem_start) return;

	uint32_t first_frame = (start - mem_start) / PAGE_SIZE;
	uint32_t frame_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;

	for (uint32_t i = 0; i < frame_count; i++)
		pfa_set_bit(first_frame + i);
}

size_t pmsize(void)
{
	return PAGE_SIZE;
}