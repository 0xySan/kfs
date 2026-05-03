#include "../includes/kernel.h"

static uint32_t *bitmap;       // pointer to where bitmap lives in RAM
static uint32_t  bitmap_size;  // number of uint32_t entries
static uint32_t  mem_start;    // first managed frame address
static uint32_t  total_frames; // total frames being tracked

static void pfa_clear_bit(uint32_t frame_index)
{
	bitmap[frame_index / 32] &= ~(1 << (frame_index % 32));
}

static void pfa_set_bit(uint32_t frame_index)
{
	bitmap[frame_index / 32] |= (1 << (frame_index % 32));
}

void *pfa_alloc_frame(void)
{
    for (uint32_t i = 0; i < bitmap_size; i++)
    {
        if (bitmap[i] == 0xFFFFFFFF)
            continue; // no free frame in this uint32_t, skip
        for (uint32_t j = 0; j < 32; j++)
        {
            if (!(bitmap[i] & (1 << j)))
            {
                pfa_set_bit(i * 32 + j);
                return (void *)(uintptr_t)(mem_start + (i * 32 + j) * PAGE_SIZE);
            }
        }
    }
    return (void *)0; // out of memory
}

void pfa_free_frame(void *frame)
{
	uint32_t frame_index = ((uintptr_t)frame - mem_start) / PAGE_SIZE;
	pfa_clear_bit(frame_index);
}

void pfa_init(multiboot_info_t *mbi)
{
	if (!(mbi->flags & (1 << 6)))
	{
		printk("PFA", "no mmap from bootloader\n");
		return;
	}

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

void show_free_frames()
{
	uint32_t free_count = 0;
	for (uint32_t i = 0; i < total_frames; i++)
		if (!(bitmap[i / 32] & (1 << (i % 32))))
			free_count++;

	printk("PFA", "free frames=%u (%u MB)\n", free_count, free_count * 4 / 1024);
}