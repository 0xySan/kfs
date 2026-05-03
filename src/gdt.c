/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   gdt.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abosc <abosc@42lehavre.fr>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 20:08:46 by abosc             #+#    #+#             */
/*   Updated: 2026/05/03 20:11:00 by abosc            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/kernel.h"

void gdt_flush(void)
{
    gdtr_t gdtr;
    gdtr.limit = (sizeof(gdt_entry_t) * 7) - 1; // * 7 par ce que j'ai 7 segments dans la GDT
    gdtr.base  = 0x00000800;

    asm volatile("lgdt %0" : : "m"(gdtr));

    // Recharger les registres segment — OBLIGATOIRE après lgdt
	// ASM inline = AT&T, syntaxe GCC donc: "mov src, dst" et % devant les registres
    asm volatile(
        "mov $0x10, %ax \n"   // 0x10 = index 2 (kernel data)
        "mov %ax, %ds   \n"
        "mov %ax, %es   \n"
        "mov %ax, %fs   \n"
        "mov %ax, %gs   \n"
        "mov %ax, %ss   \n"
        "ljmp $0x08, $1f\n"   // 0x08 = index 1 (kernel code) — recharge CS
        "1:             \n"
    );
}

void gdt_set_entry(gdt_entry_t *gdt, int index,
                   uint32_t base, uint32_t limit,
                   uint8_t access, uint8_t gran)
{
	gdt[index].base_low   = (base & 0xFFFF);
    gdt[index].base_mid   = (base >> 16) & 0xFF;
    gdt[index].base_high  = (base >> 24) & 0xFF;

    gdt[index].limit_low  = (limit & 0xFFFF);
    gdt[index].granularity = (limit >> 16) & 0x0F;  // bits 16-19 de la limite
    gdt[index].granularity |= (gran & 0xF0);        // flags dans les bits hauts

    gdt[index].access = access;
}

gdt_entry_t *gdt = (gdt_entry_t *)0x00000800;

void gdt_init(void)
{
    // Index 0 — Null descriptor (obligatoire)
    gdt_set_entry(gdt, 0, 0, 0, 0, 0);

    // Index 1 — Kernel Code
    gdt_set_entry(gdt, 1, 0, 0xFFFFF, 0x9A, 0xCF);

    // Index 2 — Kernel Data
    gdt_set_entry(gdt, 2, 0, 0xFFFFF, 0x92, 0xCF);

    // Index 3 — Kernel Stack
    gdt_set_entry(gdt, 3, 0, 0xFFFFF, 0x92, 0xCF);

    // Index 4 — User Code
    gdt_set_entry(gdt, 4, 0, 0xFFFFF, 0xFA, 0xCF);

    // Index 5 — User Data
    gdt_set_entry(gdt, 5, 0, 0xFFFFF, 0xF2, 0xCF);

    // Index 6 — User Stack
    gdt_set_entry(gdt, 6, 0, 0xFFFFF, 0xF2, 0xCF);

    gdt_flush();
}