/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   define.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:29:55 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:29:56 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DEFINE_H
#define DEFINE_H

# define IDT_SIZE	256
# define MAX_MMAP_ENTRIES 64
# define VGA_WIDTH	80
# define VGA_HEIGHT	25
# define VGA_MEMORY	0xB8000
# define TAB_WIDTH 4
# define TERMINAL_SCREEN_COUNT 4
# define TERMINAL_UI_ROW 0
# define TERMINAL_CONTENT_START_ROW 1
# define TERMINAL_CONTENT_HEIGHT (VGA_HEIGHT - TERMINAL_CONTENT_START_ROW)

# define TERMINAL_BANNER_TEXT "42"
# define TERMINAL_PROMPT_TEXT "bash$ "
# define TERMINAL_PROMPT_ROW (TERMINAL_CONTENT_START_ROW + 1)
# define TERMINAL_PROMPT_COL 6

# define MULTIBOOT_MAGIC 0x2BADB002

# define NULL ((void*)0)
# define size_t uint32_t
# define uint8_t unsigned char
# define uint16_t unsigned short
# define uint32_t unsigned int
# define uint64_t unsigned long long

#ifndef _UINTPTR_T
# define _UINTPTR_T
typedef __UINTPTR_TYPE__ uintptr_t;
#endif

#ifndef _VA_LIST
# define _VA_LIST
typedef __builtin_va_list va_list;
#endif

# define true 1
# define false 0

#if defined(__STDC_VERSION__) && __STDC_VERSION__ > 201710L
#elif !defined(__cplusplus)
# define bool _Bool
# define true 1
# define false 0
#elif defined(__GNUC__) && !defined(__STRICT_ANSI__)
/* Define _Bool as a GNU extension. */
# define _Bool bool
#endif

#define PAGE_SIZE 4096
#define ALIGN_UP(addr) (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

#define MAKE_ENTRY(addr, flags)  ((addr) | (flags))

#define PAGE_PRESENT    (1 << 0)
#define PAGE_RW         (1 << 1)
#define PAGE_USER       (1 << 2)

#define USER_SPACE_START	0x00000000
#define USER_SPACE_END		0xBFFFFFFF
#define KERNEL_SPACE_START	0xC0000000
#define KERNEL_SPACE_END	0xFFFFFFFF
// 0x00000000 - 0xBFFFFFFF  → user space   (3 GB)
// 0xC0000000 - 0xFFFFFFFF  → kernel space (1 GB)

#define HEAP_START	0xC0000000
#define HEAP_MAX	0xCFFFFFFF

#define BLACK "\e[1;30m"
#define BLUE "\e[1;31m"
#define GREEN "\e[1;32m"
#define CYAN "\e[1;33m"
#define RED "\e[1;34m"
#define MAGENTA "\e[1;35m"
#define YELLOW "\e[1;36m"
#define WHITE "\e[1;37m"

#define BLACK_BG "\e[40m"
#define BLUE_BG "\e[41m"
#define GREEN_BG "\e[42m"
#define CYAN_BG "\e[43m"
#define RED_BG "\e[44m"
#define MAGENTA_BG "\e[45m"
#define YELLOW_BG "\e[46m"
#define WHITE_BG "\e[47m"

#define RESET "\e[0m"

#endif // DEFINE_H