/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kmalloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:25:09 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 20:00:08 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "memory.h"

/**
 * Simple linked list allocator for the kernel heap. Not very efficient, but it works for basic needs. The heap grows upwards from HEAP_START, and we track
 * allocated blocks with a linked list of headers. When kmalloc is called, we look for a free block that can fit the requested size. If we can't find one, we extend the heap by allocating more pages and creating a new block header for the new space. kfree marks blocks as free and coalesces adjacent free blocks.
 */
block_header_t	*heap_start = NULL;
uintptr_t		heap_end = HEAP_START;

/**
 * Extend the heap by allocating enough pages to fit a block of the given size.
 * This function ensures that all necessary page tables exist before mapping the new pages.
 * It first checks if the required page tables are present, and if not, it allocates and initializes them.
 * Then it maps the new pages for the heap, updating heap_end accordingly.
 */
static void heap_extend(size_t size)
{
	// Calculate how many pages we need to allocate to fit the requested size plus header, rounding up to the nearest page.
	size_t pages = (size + sizeof(block_header_t) + PAGE_SIZE - 1) / PAGE_SIZE;

	if (pages > (HEAP_MAX - heap_end) / PAGE_SIZE)
		kpanic("kmalloc: requested size exceeds heap limit");

	/* Ensure page tables exist for the virtual range we will map.
		Allocate and initialize any missing page-table frames first so
		they are taken from low physical memory (identity-mapped),
		preventing faults when we initialize them. */
	if (pages > 0)
	{
		uintptr_t start = heap_end;
		uintptr_t end = heap_end + pages * PAGE_SIZE;
		uint32_t start_pd = start >> 22;
		uint32_t end_pd = (end - 1) >> 22;

		// If the heap is already mapped, this loop will be skipped since the page tables will already exist.
		for (uint32_t pdi = start_pd; pdi <= end_pd; pdi++)
		{
			// If the page directory entry is not present, we need to allocate a new page table.
			if (!(pd[pdi] & PAGE_PRESENT))
			{
				void *pt_frame = pmalloc();
				if (!pt_frame)
					kpanic("kmalloc: out of physical memory (pt)");

				/* Zero the new page table. The frame should be a low
					physical address (identity-mapped) because we allocate
					page-table frames before data frames for this batch. */
				pte_t *new_pt = (pte_t *)(uintptr_t)pt_frame;
				for (int i = 0; i < 1024; i++)
					new_pt[i] = 0;

				/* Map the new page table into the page directory.
					The page table frame should be identity-mapped,
					so we can just use its physical address directly. */
				pd[pdi] = MAKE_ENTRY((pde_t)(uintptr_t)pt_frame, PAGE_PRESENT | PAGE_RW);
			}
		}
	}

	// Now that all necessary page tables exist, we can map the new pages for the heap.
	for (size_t i = 0; i < pages; i++)
	{
		// Check if we've reached the heap limit before mapping each page.
		if (heap_end >= HEAP_MAX)
			kpanic("kmalloc: heap exhausted");

		void *frame = pmalloc();
		if (!frame)
			kpanic("kmalloc: out of physical memory");
		/* Map the new page into the heap's virtual address space.
			The frame should be identity-mapped, so we can use its physical address directly. */
		vmm_map_page(heap_end, (uintptr_t)frame, PAGE_PRESENT | PAGE_RW);
		heap_end += PAGE_SIZE;
	}
}

/**
 * Allocate a block of memory of the given size from the kernel heap.
 * If the heap hasn't been initialized yet, it sets up the initial heap with one page.
 * It then searches for a free block that can fit the requested size. If it finds one,
 * it marks it as allocated and returns a pointer to the usable space after the header.
 * If no suitable block is found, it extends the heap and creates a new block for the requested size, linking it into the list of blocks.
 */
void *kmalloc(size_t size)
{
	if (size == 0)
		return NULL;

	if (size > HEAP_MAX - HEAP_START)
		kpanic("kmalloc: requested size too large");

	/* If the heap hasn't been initialized yet, set it up with one page. */
	if (!heap_start)
	{
		// map exactly one page, no size calculation needed
		void *frame = pmalloc();
		vmm_map_page(HEAP_START, (uintptr_t)frame, PAGE_PRESENT | PAGE_RW);
		heap_end = HEAP_START + PAGE_SIZE;
		heap_start = (block_header_t *)HEAP_START;
		heap_start->size = PAGE_SIZE - sizeof(block_header_t);
		heap_start->free = 1;
		heap_start->next = NULL;
	}

	/* Find a free block that can fit the requested size. */
	block_header_t *current = heap_start;
	while (current)
	{
		if (current->free && current->size >= size)
		{
			current->free = 0;
			return (void *)(current + 1);
		}
		if (!current->next)
			break;
		current = current->next;
	}

	/* No suitable block found, need to extend the heap. */
	heap_extend(size);
	block_header_t *new_block = (block_header_t *)((uintptr_t)(current + 1) + current->size);
	new_block->size = (((size + sizeof(block_header_t) + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE) - sizeof(block_header_t);
	new_block->free = 0;
	new_block->next = NULL;

	/* Link the new block into the list. If current is NULL,
		this means the heap was empty and new_block becomes the first block.
		Otherwise, we append it to the end of the list. */
	if (!current)
		heap_start = new_block;
	else
		current->next = new_block;

	return (void *)(new_block + 1);
}

size_t kmsize(void *ptr)
{
	if (!ptr)
		return 0;

	block_header_t *header = (block_header_t *)ptr - 1;
	return header->size;
}

void kfree(void *ptr)
{
	if (!ptr)
		return;

	block_header_t *header = (block_header_t *)ptr - 1;
	header->free = 1;

	if (header->next && header->next->free)
	{
		header->size += sizeof(block_header_t) + header->next->size;
		header->next = header->next->next;
	}
}