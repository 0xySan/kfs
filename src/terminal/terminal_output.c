/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   terminal_output.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:26:06 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:32:55 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "terminal.h"

// terminal_output.c - Text output and writing functions

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	struct terminal_screen* screen = terminal_current_screen();
	volatile uint16_t* const vga_buffer = (uint16_t*)VGA_MEMORY;
	const size_t index = y * VGA_WIDTH + x;

	screen->cells[index] = vga_entry(c, color);
	screen->is_tab_space[y][x] = false;
	screen->is_cell_occupied[y][x] = (c != ' ');
	vga_buffer[index] = screen->cells[index];
}

void terminal_write(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		if (data[i] == '\033' && i + 1 < size && data[i + 1] == '[')
		{
			terminal_handle_escape_sequence(data, &i);
			continue;
		}
		if (data[i] == '\t')
			terminal_render_tab();
		else if (data[i] == '\n')
			terminal_render_newline();
		else if (data[i] == '\b')
			terminal_render_backspace();
		else
			terminal_render_char(data[i]);
	}
	terminal_flush_active_screen();
	terminal_update_cursor();
}

void terminal_writestring(const char* data)
{
	terminal_write(data, strlen(data));
}
