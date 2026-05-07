/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   terminal_cursor.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:25:32 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:25:33 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "terminal.h"

// terminal_cursor.c - Cursor positioning and movement

void terminal_update_cursor(void)
{
	struct terminal_screen* screen = terminal_current_screen();

	if (screen->row < TERMINAL_CONTENT_START_ROW)
		screen->row = TERMINAL_CONTENT_START_ROW;
	uint16_t pos = (uint16_t)(screen->row * VGA_WIDTH + screen->column);
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

size_t terminal_cursor_index(void)
{
	struct terminal_screen* screen = terminal_current_screen();

	return screen->row * VGA_WIDTH + screen->column;
}

void terminal_setrow(size_t row)
{
	struct terminal_screen* screen = terminal_current_screen();

	screen->row = terminal_clamp_content_row(row);
	screen->preferred_column = screen->column;
	terminal_update_cursor();
}

void terminal_setcolumn(size_t column)
{
	struct terminal_screen* screen = terminal_current_screen();

	if (column >= VGA_WIDTH)
		column = VGA_WIDTH - 1;
	screen->column = column;
	screen->preferred_column = screen->column;
	terminal_update_cursor();
}

void terminal_setcolor(uint8_t color)
{
	terminal_current_screen()->color = color;
}

void terminal_advance_cursor(void)
{
	struct terminal_screen* screen = terminal_current_screen();

	if (++screen->column == VGA_WIDTH)
	{
		screen->column = 0;
		if (++screen->row == VGA_HEIGHT)
		{
			terminal_scroll_up();
			screen->row = VGA_HEIGHT - 1;
		}
	}
}

size_t terminal_tab_span_before_cursor(size_t row, size_t column)
{
	struct terminal_screen* screen = terminal_current_screen();
	size_t start;

	if (column == 0 || !screen->is_tab_space[row][column - 1])
		return 1;
	start = column - 1;
	while (start > 0
		&& screen->is_tab_space[row][start - 1]
		&& (start % TAB_WIDTH) != 0)
		start--;
	return column - start;
}

size_t terminal_tab_span_after_cursor(size_t row, size_t column, size_t max_col)
{
	struct terminal_screen* screen = terminal_current_screen();
	size_t end;

	if (column >= max_col || !screen->is_tab_space[row][column])
		return 1;
	end = column + 1;
	while (end < max_col
		&& screen->is_tab_space[row][end]
		&& (end % TAB_WIDTH) != 0)
		end++;
	return end - column;
}

void handle_arrow_keys(uint8_t arrow_key)
{
	struct terminal_screen* screen = terminal_current_screen();
	size_t min_col = terminal_locked_prefix_col(screen->row);

	if (arrow_key == 0x4D) // Right arrow
	{
		size_t max_col = terminal_row_max_cursor_col(screen->row);
		if (screen->column < max_col)
			screen->column += terminal_tab_span_after_cursor(screen->row,
					screen->column, max_col);
	}
	else if (arrow_key == 0x4B) // Left Arrow
	{
		if (screen->column > min_col)
		{
			size_t span = terminal_tab_span_before_cursor(screen->row, screen->column);
			if (span > screen->column - min_col)
				span = screen->column - min_col;
			screen->column -= span;
		}
	}
	terminal_update_cursor();
}
