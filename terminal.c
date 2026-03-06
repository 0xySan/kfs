#include "kernel.h"

uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
static size_t terminal_preferred_column;

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

static size_t terminal_cursor_index(void)
{
	return terminal_row * VGA_WIDTH + terminal_column;
}

static void terminal_scroll_up(void)
{
	for (size_t y = 1; y < VGA_HEIGHT; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
			terminal_buffer[(y - 1) * VGA_WIDTH + x] = terminal_buffer[y * VGA_WIDTH + x];
	}
	for (size_t x = 0; x < VGA_WIDTH; x++)
		terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
}

static void terminal_advance_cursor(void)
{
	if (++terminal_column == VGA_WIDTH)
	{
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
		{
			terminal_scroll_up();
			terminal_row = VGA_HEIGHT - 1;
		}
	}
}

static void terminal_insert_char(char c)
{
	size_t row_base = terminal_row * VGA_WIDTH;
	size_t cursor = row_base + terminal_column;
	size_t row_end = row_base + VGA_WIDTH - 1;

	for (size_t i = row_end; i > cursor; i--)
		terminal_buffer[i] = terminal_buffer[i - 1];
	terminal_buffer[cursor] = vga_entry(c, terminal_color);
	terminal_advance_cursor();
}


static int terminal_row_last_filled_col(size_t row)
{
	for (size_t x = VGA_WIDTH; x > 0; x--)
	{
		uint16_t entry = terminal_buffer[row * VGA_WIDTH + (x - 1)];
		if ((char)(entry & 0xFF) != ' ')
			return (int)(x - 1);
	}
	return -1;
}

static size_t terminal_row_max_cursor_col(size_t row)
{
	int last_col = terminal_row_last_filled_col(row);

	if (last_col < 0)
		return 0;
	if ((size_t)last_col < VGA_WIDTH - 1)
		return (size_t)last_col + 1;
	return VGA_WIDTH - 1;
}

static void terminal_delete_char_before_cursor(void)
{
	size_t row_base;
	size_t cursor;
	size_t row_end;

	if (terminal_row == 0 && terminal_column == 0)
		return;
	if (terminal_column > 0)
		terminal_column--;
	else
	{
		terminal_row--;
		terminal_column = terminal_row_max_cursor_col(terminal_row) + 1;
		if (terminal_column > 0)
			terminal_column--;
		return;
	}
	row_base = terminal_row * VGA_WIDTH;
	cursor = terminal_cursor_index();
	row_end = row_base + VGA_WIDTH - 1;
	for (size_t i = cursor; i < row_end; i++)
		terminal_buffer[i] = terminal_buffer[i + 1];
	terminal_buffer[row_end] = vga_entry(' ', terminal_color);
}

void terminal_initialize(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_preferred_column = 0;
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
	if (row >= VGA_HEIGHT)
		row = VGA_HEIGHT - 1;
	terminal_row = row;
	terminal_preferred_column = terminal_column;
	terminal_update_cursor();
}

void terminal_setcolumn(size_t column)
{
	if (column >= VGA_WIDTH)
		column = VGA_WIDTH - 1;
	terminal_column = column;
	terminal_preferred_column = terminal_column;
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
	terminal_insert_char(c);
}

void handle_backspace(void)
{
	terminal_delete_char_before_cursor();
	terminal_preferred_column = terminal_column;
	terminal_update_cursor();
}

void handle_arrow_keys(uint8_t arrow_key)
{
	if (arrow_key == 0x4D) // Right arrow
	{
		size_t max_col = terminal_row_max_cursor_col(terminal_row);
		if (terminal_column < max_col)
			terminal_column++;
	}
	else if (arrow_key == 0x4B) // Left Arrow
	{
		if (terminal_column > 0)
			terminal_column--;
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
			if (++terminal_row == VGA_HEIGHT)
			{
				terminal_scroll_up();
				terminal_row = VGA_HEIGHT - 1;
			}
		}
		else if (data[i] == 0x08)
			handle_backspace();
		else
			terminal_insert_char(data[i]);
	}
	terminal_preferred_column = terminal_column;
	terminal_update_cursor();
}

void terminal_writestring(const char* data)
{
	terminal_write(data, ft_strlen(data));
}
