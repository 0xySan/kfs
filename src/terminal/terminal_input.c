/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   terminal_input.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:25:36 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:25:37 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "terminal.h"

// terminal_input.c - Input handling and text insertion

void terminal_collect_current_command(char* out, size_t out_size)
{
	struct terminal_screen* screen = terminal_current_screen();
	size_t row = screen->row;
	size_t start = terminal_locked_prefix_col(row);
	int last_col = terminal_row_last_filled_col(row);
	size_t out_i = 0;

	if (out_size == 0)
		return;
	if (last_col < 0 || (size_t)last_col < start)
	{
		out[0] = '\0';
		return;
	}
	while ((size_t)last_col >= start)
	{
		char c = (char)(screen->cells[row * VGA_WIDTH + (size_t)last_col] & 0xFF);
		if (c != ' ')
			break;
		last_col--;
	}
	if (last_col < 0 || (size_t)last_col < start)
	{
		out[0] = '\0';
		return;
	}
	for (size_t x = start; x <= (size_t)last_col && out_i + 1 < out_size; x++)
		out[out_i++] = (char)(screen->cells[row * VGA_WIDTH + x] & 0xFF);
	out[out_i] = '\0';
}

void terminal_insert_char(char c)
{
	struct terminal_screen* screen = terminal_current_screen();
	size_t row_base = screen->row * VGA_WIDTH;
	size_t cursor = row_base + screen->column;
	size_t row_end = row_base + VGA_WIDTH - 1;

	for (size_t i = row_end; i > cursor; i--)
		screen->cells[i] = screen->cells[i - 1];
	screen->cells[cursor] = vga_entry(c, screen->color);

	for (size_t x = VGA_WIDTH - 1; x > screen->column; x--)
	{
		screen->is_tab_space[screen->row][x] = screen->is_tab_space[screen->row][x - 1];
		screen->is_cell_occupied[screen->row][x] = screen->is_cell_occupied[screen->row][x - 1];
	}
	screen->is_tab_space[screen->row][screen->column] = false;
	screen->is_cell_occupied[screen->row][screen->column] = true;

	terminal_advance_cursor();
}

void terminal_delete_char_before_cursor(void)
{
	struct terminal_screen* screen = terminal_current_screen();
	size_t row_base;
	size_t cursor;
	size_t row_end;
	size_t chars_to_delete = 1;
	size_t min_col;

	if (screen->column == 0)
	{
		if (screen->row > TERMINAL_CONTENT_START_ROW)
		{
			int last_col;
			size_t prev_min_col;

			screen->row--;
			prev_min_col = terminal_locked_prefix_col(screen->row);
			last_col = terminal_row_last_filled_col(screen->row);
			if (last_col < 0 || ((size_t)last_col + 1) <= prev_min_col)
			{
				screen->column = prev_min_col;
				return;
			}
			if ((size_t)last_col == VGA_WIDTH - 1)
			{
				size_t col = VGA_WIDTH - 1;
				size_t index = screen->row * VGA_WIDTH + col;

				screen->cells[index] = vga_entry(' ', screen->color);
				screen->is_tab_space[screen->row][col] = false;
				screen->is_cell_occupied[screen->row][col] = false;
				screen->column = col;
				return;
			}
			screen->column = (size_t)last_col + 1;
		}
		return;
	}
	min_col = terminal_locked_prefix_col(screen->row);
	if (screen->column <= min_col)
		return;
	if (screen->column > 0)
	{
		chars_to_delete = terminal_tab_span_before_cursor(screen->row, screen->column);
		if (chars_to_delete > screen->column - min_col)
			chars_to_delete = screen->column - min_col;
		for (size_t j = 0; j < chars_to_delete; j++)
		{
			screen->is_tab_space[screen->row][screen->column - chars_to_delete + j] = false;
			screen->is_cell_occupied[screen->row][screen->column - chars_to_delete + j] = false;
		}
		screen->column -= chars_to_delete;
	}
	row_base = screen->row * VGA_WIDTH;
	cursor = terminal_cursor_index();
	row_end = row_base + VGA_WIDTH - 1;
	for (size_t i = cursor; i + chars_to_delete <= row_end; i++)
	{
		screen->cells[i] = screen->cells[i + chars_to_delete];
		screen->is_tab_space[screen->row][i - row_base] = screen->is_tab_space[screen->row][i - row_base + chars_to_delete];
		screen->is_cell_occupied[screen->row][i - row_base] = screen->is_cell_occupied[screen->row][i - row_base + chars_to_delete];
	}
	for (size_t i = 0; i < chars_to_delete; i++)
	{
		screen->cells[row_end - i] = vga_entry(' ', screen->color);
		screen->is_tab_space[screen->row][VGA_WIDTH - 1 - i] = false;
		screen->is_cell_occupied[screen->row][VGA_WIDTH - 1 - i] = false;
	}
}

void handle_backspace(void)
{
	struct terminal_screen* screen = terminal_current_screen();

	terminal_delete_char_before_cursor();
	screen->preferred_column = screen->column;
	terminal_flush_active_screen();
	terminal_update_cursor();
}

void terminal_newline_with_prompt(void)
{
	struct terminal_screen* screen = terminal_current_screen();
	char command[128];

	terminal_collect_current_command(command, sizeof(command));

	screen->column = 0;
	if (++screen->row == VGA_HEIGHT)
	{
		terminal_scroll_up();
		screen->row = VGA_HEIGHT - 1;
	}
	screen->locked_prefix_col[screen->row] = 0;

	execute_command(command);
	if (screen->column != 0)
	{
		screen->column = 0;
		if (++screen->row == VGA_HEIGHT)
		{
			terminal_scroll_up();
			screen->row = VGA_HEIGHT - 1;
		}
		screen->locked_prefix_col[screen->row] = 0;
	}
	screen->locked_prefix_col[screen->row] = (uint8_t)strlen(TERMINAL_PROMPT_TEXT);
	for (size_t i = 0; TERMINAL_PROMPT_TEXT[i] != '\0'; i++)
		terminal_insert_char(TERMINAL_PROMPT_TEXT[i]);
}

void terminal_write_input(const char* data, size_t size)
{
	struct terminal_screen* screen = terminal_current_screen();

	if (screen->row < TERMINAL_CONTENT_START_ROW)
		screen->row = TERMINAL_CONTENT_START_ROW;

	for (size_t i = 0; i < size; i++)
	{
		if (data[i] == '\t')
		{
			size_t tab_start_row = screen->row;
			size_t tab_start_col = screen->column;
			size_t tab_width = TAB_WIDTH - (screen->column % TAB_WIDTH);
			for (size_t j = 0; j < tab_width; j++)
				terminal_insert_char(' ');
			if (screen->row == tab_start_row)
			{
				for (size_t j = 0; j < tab_width; j++)
					screen->is_tab_space[tab_start_row][tab_start_col + j] = true;
			}
		}
		else if (data[i] == '\n')
			terminal_newline_with_prompt();
		else if (data[i] == '\b')
			handle_backspace();
		else
			terminal_insert_char(data[i]);
	}
	screen->preferred_column = screen->column;
	terminal_flush_active_screen();
	terminal_update_cursor();
}
