#ifndef KERNEL_H
# define KERNEL_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include <stdarg.h>

# define IDT_SIZE	256
# define VGA_WIDTH	80
# define VGA_HEIGHT	25
# define VGA_MEMORY	0xB8000
# define TAB_WIDTH 4

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

static const char scancode_map[128] = {
	0,   0,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'-', '=', 0x08,   '\t',  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
	'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd', 'f', 'g', 'h',
	'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
	'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

static const char shift_scancode_map[128] = {
	0,   0,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'_', '+', 0x08,   '\t',  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
	'O', 'P', '{', '}', '\n', 0, 'A', 'S', 'D', 'F', 'G', 'H',
	'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', '<' , '>' , '?', 0, '*', 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static inline void outb(uint16_t port, uint8_t value)
{
	__asm__ volatile ("outb %0, %1"
		:
		: "a"(value), "Nd"(port)
	);
}

static inline uint8_t inb(uint16_t port)
{
	uint8_t value;
	__asm__ volatile ("inb %1, %0"
		: "=a"(value)
		: "Nd"(port)
	);
	return value;
}

static inline void io_wait(void)
{
	outb(0x80, 0);
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

void load_idt(struct idt_ptr *ptr);
void keyboard_handler_stub(void);
void keyboard_handler(void);
void idt_set_gate(uint8_t num, uint32_t handler_address, 
				  uint16_t selector, uint8_t type_attr);
void idt_init(void);
void pic_init(void);
void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_setrow(size_t row);
void terminal_setcolumn(size_t column);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void handle_backspace(void);
char scancode_to_ascii(uint8_t scancode);
int kprintf(const char *format, ...);
int printk(const char *level, const char *format, ...);
void dump_kernel_stack(size_t words);
void handle_arrow_keys(uint8_t arrow_key);

#endif
