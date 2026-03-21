#ifndef DEFINE_H
#define DEFINE_H

# define IDT_SIZE	256
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

#define NULL ((void*)0)
#define size_t uint32_t
#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int

#ifndef _UINTPTR_T
typedef __UINTPTR_TYPE__ uintptr_t;
#define _UINTPTR_T
#endif

#ifndef _VA_LIST
#define _VA_LIST
typedef __builtin_va_list va_list;
#endif

#define true 1
#define false 0
#define bool _Bool

#endif // DEFINE_H