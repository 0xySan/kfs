/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kmalloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:25:09 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/09 21:55:04 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "memory.h"

/*
** Electric-fence style heap allocator.
**
** Each allocation is placed at the END of its mapped page(s), immediately
** before an unmapped guard page. Any write past the allocation hits the
** guard page and triggers a page fault instantly.
**
** Layout for kmalloc(17):
**
**   page N:   [padding ...][header 12B][data 17B]  <- mapped
**   page N+1: [guard - UNMAPPED]                   <- page fault on overflow
**
** On kfree, the data pages are unmapped so use-after-free also faults.
** On realloc (reuse), the pages are remapped before returning.
**
** Virtual memory layout:
**   HEAP_START (0xC0000000) .. HEAP_MAX (0xCFFFFFFF)  kernel heap (255 MB)
*/

block_header_t	*heap_start = NULL;
uintptr_t		heap_end = HEAP_START;

/* ── helpers ────────────────────────────────────────────────────────────── */

/*
** Ensure the page directory has a page table for every 4-MB region that
** covers [start, end). Allocates and zeros missing PT frames from low
** physical memory (identity-mapped) so we can safely initialise them
** before paging those virtual addresses.
*/
static void ensure_page_tables(uintptr_t start, uintptr_t end)
{
	uint32_t pdi;

	for (pdi = start >> 22; pdi <= (end - 1) >> 22; pdi++)
	{
		if (!(pd[pdi] & PAGE_PRESENT))
		{
			pte_t *new_pt = (pte_t *)(uintptr_t)pmalloc();
			if (!new_pt)
				kpanic("kmalloc: out of physical memory (pt)", NULL);
			for (int i = 0; i < 1024; i++)
				new_pt[i] = 0;
			pd[pdi] = MAKE_ENTRY((pde_t)(uintptr_t)new_pt,
								 PAGE_PRESENT | PAGE_RW);
		}
	}
}

/*
** Map physical frames for every page in [page_start, page_end).
** page_start and page_end must already be page-aligned.
*/
static void map_pages(uintptr_t page_start, uintptr_t page_end)
{
	uintptr_t addr;

	ensure_page_tables(page_start, page_end);
	for (addr = page_start; addr < page_end; addr += PAGE_SIZE)
	{
		void *frame = pmalloc();
		if (!frame)
			kpanic("kmalloc: out of physical memory", NULL);
		vmm_map_page(addr, (uintptr_t)frame, PAGE_PRESENT | PAGE_RW);
	}
}

/*
** Unmap (but do not free the physical frames of) every page that holds
** the block's data. Used by kfree to catch use-after-free.
*/
static void unmap_block_pages(block_header_t *header)
{
	// header page must stay mapped for linked list traversal
	uintptr_t header_page = ALIGN_DOWN((uintptr_t)header);
	uintptr_t data_start  = ALIGN_UP((uintptr_t)(header + 1));
	uintptr_t data_end    = ALIGN_UP((uintptr_t)(header + 1) + header->size);
	uintptr_t addr;

	// only unmap pages that don't contain the header
	for (addr = data_start; addr < data_end; addr += PAGE_SIZE)
	{
		if (addr != header_page)
			vmm_unmap_page(addr);
	}
}

/*
** Remap pages for a previously freed (unmapped) block so it can be reused.
*/
static void remap_block_pages(block_header_t *header)
{
	uintptr_t data_start = ALIGN_DOWN((uintptr_t)(header + 1));
	uintptr_t data_end   = ALIGN_UP((uintptr_t)(header + 1) + header->size);

	map_pages(data_start, data_end);
}

/*
** Allocate a brand-new block of exactly `size` usable bytes using the
** electric-fence layout:
**
**   [ page-aligned base ]
**       ...padding...
**       [ block_header_t ]   <- header placed just before data end
**       [ size bytes     ]   <- data ends exactly at page_end
**   [ page_end = guard   ]   <- unmapped
**
** Updates heap_end to page_end + PAGE_SIZE (past the guard).
** Returns a pointer to the block header (NOT the usable data).
*/
static block_header_t *alloc_new_block(size_t size)
{
	uintptr_t	base;
	uintptr_t	page_end;
	uintptr_t	data_start;
	block_header_t	*block;

	base     = heap_end;
	page_end = ALIGN_UP(base + sizeof(block_header_t) + size);

	if (page_end + PAGE_SIZE > HEAP_MAX)
		kpanic("kmalloc: heap exhausted", NULL);

	/* Map data pages (guard page at page_end is intentionally left unmapped) */
	map_pages(base, page_end);

	/* Place header immediately before the data, data ends at page_end */
	data_start = page_end - size;
	block = (block_header_t *)(data_start - sizeof(block_header_t));

	block->size = size;   /* exact requested size */
	block->free = 0;
	block->next = NULL;

	/* Advance heap_end past the guard page */
	heap_end = page_end + PAGE_SIZE;

	return block;
}

/* ── public API ─────────────────────────────────────────────────────────── */

void *kmalloc(size_t size)
{
	if (size == 0)
		return NULL;
	if (size > HEAP_MAX - HEAP_START)
		kpanic("kmalloc: requested size too large", NULL);

	block_header_t *current = heap_start;
	while (current)
	{
		if (current->free && current->size >= size)
		{
			current->free = 0;
			// only remap if pages were unmapped by kfree
			// check if the page is actually mapped first
			if (!vmm_get_physical(ALIGN_DOWN((uintptr_t)(current + 1))))
				remap_block_pages(current);
			return (void *)(current + 1);
		}
		if (!current->next)
			break;
		current = current->next;
	}

	block_header_t *new_block = alloc_new_block(size);
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
	return ((block_header_t *)ptr - 1)->size;
}

void kfree(void *ptr)
{
	if (!ptr)
		return;

	block_header_t *header = (block_header_t *)ptr - 1;
	header->free = 1;

	unmap_block_pages(header);
}