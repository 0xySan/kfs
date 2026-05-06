#include "../includes/kernel.h"

// VMM (Virtual Memory Manager) - manages virtual addresses, maps them to physical frames using the page tables

/* Maps the given physical address to the given virtual address with the specified flags.
   If the page table for the virtual address doesn't exist, it will be created. */
void vmm_map_page(uintptr_t virt, uintptr_t phys, uint32_t flags)
{
	uint32_t pd_index = virt >> 22;
	uint32_t pt_index = (virt >> 12) & 0x3FF;

	if (!(pd[pd_index] & PAGE_PRESENT))
	{
		pde_t new_pt_addr = (pde_t)(uintptr_t)pfa_alloc_frame();
		if (!new_pt_addr)
			kpanic("vmm_map_page: out of physical memory");
		pte_t *new_pt = (pte_t *)(uintptr_t)new_pt_addr;
		for (int i = 0; i < 1024; i++)
			new_pt[i] = 0;
		pd[pd_index] = MAKE_ENTRY(new_pt_addr, PAGE_PRESENT | PAGE_RW | PAGE_USER);
	}

	pte_t *pt_base = (pte_t *)(uintptr_t)(pd[pd_index] & ~0xFFF); // Get the base address of the page table removing the flags
	pt_base[pt_index] = MAKE_ENTRY(phys, flags);

	__asm__ volatile ("invlpg (%0)" : : "r"(virt) : "memory");
}

/*	Unmaps the page at the given virtual address. Does nothing if not mapped. 
	Note: this does not free the physical frame, it only removes the mapping.
	The caller is responsible for calling pfa_free_frame() if they want to release the physical memory too.*/
void vmm_unmap_page(uintptr_t virt)
{
	uint32_t pd_index = virt >> 22;
	uint32_t pt_index = (virt >> 12) & 0x3FF;

	if (!(pd[pd_index] & PAGE_PRESENT))
		return;

	pte_t *pt_base = (pte_t *)(uintptr_t)(pd[pd_index] & ~0xFFF);
	pt_base[pt_index] = 0;

	__asm__ volatile ("invlpg (%0)" : : "r"(virt) : "memory");
}

/* Returns the physical address mapped to the given virtual address, or 0 if not mapped */
uintptr_t vmm_get_physical(uintptr_t virt)
{
	uint32_t pd_index = virt >> 22;
	uint32_t pt_index = (virt >> 12) & 0x3FF;

	if (!(pd[pd_index] & PAGE_PRESENT))
		return 0;

	pte_t *pt_base = (pte_t *)(uintptr_t)(pd[pd_index] & ~0xFFF);

	if (!(pt_base[pt_index] & PAGE_PRESENT))
		return 0;

	return (pt_base[pt_index] & ~0xFFF) | (virt & 0xFFF);
}