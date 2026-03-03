#include "idt.h"

uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

static size_t ft_strlen(const char* str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

static void terminal_update_cursor(void)
{
	uint16_t pos = (uint16_t)(terminal_row * VGA_WIDTH + terminal_column);
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void terminal_initialize(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
	terminal_update_cursor();
}

void terminal_setrow(size_t row)
{
	terminal_row = row;
	terminal_update_cursor();
}

void terminal_setcolumn(size_t column)
{
	terminal_column = column;
	terminal_update_cursor();
}

void terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c)
{
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH)
	{
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}

void handle_backspace(void)
{
	if (terminal_column > 0) {
		terminal_column--;
		terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
	} else if (terminal_row > 0) {
		size_t last_column = 0;
		int found = 0;

		terminal_row--;
		for (size_t x = VGA_WIDTH; x > 0; x--)
		{
			uint16_t entry = terminal_buffer[terminal_row * VGA_WIDTH + (x - 1)];
			if ((char)(entry & 0xFF) != ' ')
			{
				last_column = x - 1;
				found = 1;
				break;
			}
		}

		if (found)
			terminal_column = last_column;
		else
			terminal_column = 0;

		terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
	}
	terminal_update_cursor();
}

void terminal_write(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		if (data[i] == '\n')
		{
			terminal_column = 0;
			terminal_row++;
		}
		else if (data[i] == '\t')
			terminal_column += 4;
		else if (data[i] == 0x08)
			handle_backspace();
		else
			terminal_putchar(data[i]);

		if (terminal_row == VGA_HEIGHT)
			terminal_row = 0;
		if (terminal_column >= VGA_WIDTH)
		{
			terminal_column = 0;
			if (++terminal_row == VGA_HEIGHT)
				terminal_row = 0;
		}
	}
	terminal_update_cursor();
}

void terminal_writestring(const char* data)
{
	terminal_write(data, ft_strlen(data));
}
