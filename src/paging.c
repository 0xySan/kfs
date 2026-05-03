/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   paging.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abosc <abosc@42lehavre.fr>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 21:13:44 by abosc             #+#    #+#             */
/*   Updated: 2026/05/03 22:47:53 by abosc            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/kernel.h"

pde_t pd[1024] __attribute__((aligned(4096)));
pte_t pt[1024] __attribute__((aligned(4096)));

/*
** Virtual Memory Layout
** =====================
** 0x00000000 - 0xBFFFFFFF : User space   (3 GB)
**   - user code, stack, heap
**   - flags: PAGE_PRESENT | PAGE_RW | PAGE_USER
**
** 0xC0000000 - 0xFFFFFFFF : Kernel space (1 GB)
**   - kernel code, data, heap
**   - flags: PAGE_PRESENT | PAGE_RW (no PAGE_USER)
**
** Physical memory starts at 0x100000 (1MB)
** Kernel loaded at 0x100000
** Bitmap PFA starts after kernel_end
*/
void paging_init()
{
	for (int i = 0; i < 1024; i++)
	{
		pd[i] = 0;
		pt[i] = 0;
	}
	for (int i = 0; i < 1024; i++)
		pt[i] = MAKE_ENTRY(i * PAGE_SIZE, PAGE_PRESENT | PAGE_RW);

	pd[0] = MAKE_ENTRY((pde_t)(uintptr_t)pt, PAGE_PRESENT | PAGE_RW);
	__asm__ volatile ("mov %0, %%cr3" : : "r"((uintptr_t)pd));

	uint32_t cr0;
	__asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 1 << 31;
	__asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
}