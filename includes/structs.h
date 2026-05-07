/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   structs.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:29:32 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 23:54:01 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STRUCTS_H
# define STRUCTS_H

#include "define.h"

struct terminal_screen {
	uint16_t cells[VGA_HEIGHT * VGA_WIDTH];
	bool is_tab_space[VGA_HEIGHT][VGA_WIDTH];
	bool is_cell_occupied[VGA_HEIGHT][VGA_WIDTH];
	uint8_t locked_prefix_col[VGA_HEIGHT];
	size_t row;
	size_t column;
	size_t preferred_column;
	uint8_t color;
	uint8_t ansi_bold;
    /* Command history */
    int history_count;
    int history_next; /* next insertion index */
    int history_nav; /* -1 when not navigating, otherwise nav offset (0 = last) */
    char history_temp[128]; /* temporary storage of current input when navigating */
    char history[32][128];
};

struct idt_entry {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t  zero;
	uint8_t  type_attr;
	uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

typedef struct {
	// +-------------------+		(required) 0

	uint32_t flags;					// which fields below are valid

	// +-------------------+		(present if flags[0] is set) 4 & 8

	uint32_t mem_lower;				// KB of lower memory (below 1MB)
	uint32_t mem_upper;				// KB of upper memory (above 1MB)

	// +-------------------+		(present if flags[1] is set) 12

	uint32_t boot_device;			// BIOS disk device the OS image was loaded from

	// +-------------------+		(present if flags[2] is set) 16

	uint32_t cmdline;				// Address of the kernel command line

	// +-------------------+		(present if flags[3] is set) 20 & 24

	uint32_t mods_count;			// Number of boot modules loaded
	uint32_t mods_addr;				// Physical address of the first module structure

	// +-------------------+		(present if flags[4] or flags[5] is set) 28 - 40

	uint32_t syms[4];				// If bits 4 & 5 are clear, these hold the a.out symbol table. Otherwise, they hold the ELF section header table.

	// +-------------------+		(present if flags[6] is set) 44 & 48

	uint32_t mmap_length;			// size in bytes of the mmap buffer
	uint32_t mmap_addr;				// physical address of mmap entries

	// +-------------------+		(present if flags[7] is set) 52 & 56

	uint32_t drives_length;			// size in bytes of the drives buffer
	uint32_t drives_addr;			// physical address of the first drive structure

	// +-------------------+		(present if flags[8] is set) 60

	uint32_t config_table;			// address of the ROM configuration table

	// +-------------------+		(present if flags[9] is set) 64

	uint32_t boot_loader_name;		// address of a string containing the name of the bootloader

	// +-------------------+		(present if flags[10] is set) 68

	uint32_t apm_table;				// address of the APM table

	// +-------------------+		(present if flags[11] is set) 72 & 76 & 80 & 82 & 84 & 86

	uint32_t vbe_control_info;		// address of the VBE control information returned by the VBE function 0x4F00
	uint32_t vbe_mode_info;			// address of the VBE mode information returned by the VBE function 0x4F01
	uint16_t vbe_mode;				// VBE mode selected by int 0x10 function 0x4F02
	uint16_t vbe_interface_seg;		// segment of VBE interface
	uint16_t vbe_interface_off;		// offset of VBE interface
	uint16_t vbe_interface_len;		// length of VBE interface 

	// +-------------------+		(present if flags[12] is set) 88 & 92 & 96 & 100 & 104 & 108 & 109 & 110-115

	uint64_t framebuffer_addr;		// physical address of the framebuffer
	uint32_t framebuffer_pitch;		// number of bytes to write to move down a line
	uint32_t framebuffer_width;		// width of the framebuffer in pixels
	uint32_t framebuffer_height;	// height of the framebuffer in pixels
	uint8_t  framebuffer_bpp;		// bits per pixel
	uint8_t  framebuffer_type;		// type of framebuffer (0: indexed color, 1: direct RGB, 2: EGA text)
	uint16_t color_info;			// reserved for future use

	// +-------------------+ 		(end of structure, may be padded to a multiple of 8 or 16 bytes)
} multiboot_info_t;

typedef struct {
	uint32_t size;		// size of THIS entry (not including this field)
	uint64_t addr;		// base address of region
	uint64_t len;		// length in bytes
	uint32_t type;		// 1 = available RAM, anything else = reserved
} __attribute__((packed)) multiboot_mmap_entry_t;

typedef struct {
	uint16_t limit_low;    // bits 0-15 de la limite
	uint16_t base_low;     // bits 0-15 de la base
	uint8_t  base_mid;     // bits 16-23 de la base
	uint8_t  access;       // droits d'accès (le plus important)
	uint8_t  granularity;  // flags + bits 16-19 de la limite
	uint8_t  base_high;    // bits 24-31 de la base
} __attribute__((packed)) gdt_entry_t;

typedef struct {
	uint16_t limit;   // taille totale de la GDT - 1
	uint32_t base;    // adresse de la GDT
} __attribute__((packed)) gdtr_t;

typedef uint32_t pde_t;   // Page Directory Entry
typedef uint32_t pte_t;   // Page Table Entry

typedef struct block_header
{
	size_t              size;
	int                 free;
	struct block_header *next;
} block_header_t;

typedef struct registers_s
{
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_num;
	uint32_t error_code;
	uint32_t eip, cs, eflags;
} registers_t;

typedef uint32_t (*syscall_fn_t)(registers_t *regs);

#endif // STRUCTS_H