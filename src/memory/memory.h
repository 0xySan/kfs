#ifndef MEMORY_H
# define MEMORY_H

#include "../../includes/structs.h"
#include "../../includes/extern.h"

extern block_header_t	*heap_start;
extern uintptr_t		heap_end;

// Paging interface
void paging_init();

// PFA (Physical Frame Allocator) interface
void *pmalloc(void);
void pfa_init(multiboot_info_t *mbi);
void pfree(void *frame);
void pfa_mark_used(uintptr_t start, size_t size);

// VMM (Virtual Memory Manager) interface
void vmm_map_page(uintptr_t virt, uintptr_t phys, uint32_t flags);
void vmm_unmap_page(uintptr_t virt);
uintptr_t vmm_get_physical(uintptr_t virt);

// kmalloc interface
void *kmalloc(size_t size);
size_t kmsize(void *ptr);
void kfree(void *ptr);

// needed from kernel.c to avoid circular include
void kpanic(const char *msg);
void kwarn(const char *msg);

#endif /* MEMORY_H */