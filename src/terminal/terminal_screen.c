/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   terminal_screen.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:26:16 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 22:57:56 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "terminal.h"

// terminal_screen.c - Screen management and buffer operations
static volatile uint16_t* const vga_buffer = (uint16_t*)VGA_MEMORY;
static struct terminal_screen screens[TERMINAL_SCREEN_COUNT];
static size_t active_screen;

struct terminal_screen* terminal_current_screen(void)
{
	return &screens[active_screen];
}

size_t terminal_get_active_screen(void)
{
	return active_screen;
}

void terminal_switch_screen(size_t screen_index)
{
	if (screen_index >= TERMINAL_SCREEN_COUNT)
		return;
	active_screen = screen_index;
	terminal_redraw_status_bars();
	terminal_flush_active_screen();
	terminal_update_cursor();
}

void terminal_flush_active_screen(void)
{
	struct terminal_screen* screen = terminal_current_screen();

	for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++)
		vga_buffer[i] = screen->cells[i];
}

size_t terminal_locked_prefix_col(size_t row)
{
	return terminal_current_screen()->locked_prefix_col[row];
}

int terminal_row_last_filled_col(size_t row)
{
	struct terminal_screen* screen = terminal_current_screen();

	for (size_t x = VGA_WIDTH; x > 0; x--)
	{
		uint16_t entry = screen->cells[row * VGA_WIDTH + (x - 1)];
		if (screen->is_cell_occupied[row][x - 1]
			|| (char)(entry & 0xFF) != ' '
			|| screen->is_tab_space[row][x - 1])
			return (int)(x - 1);
	}
	return -1;
}

size_t terminal_row_max_cursor_col(size_t row)
{
	int last_col = terminal_row_last_filled_col(row);

	if (last_col < 0)
		return 0;
	if ((size_t)last_col < VGA_WIDTH - 1)
		return (size_t)last_col + 1;
	return VGA_WIDTH - 1;
}

size_t terminal_clamp_content_row(size_t row)
{
	if (row >= TERMINAL_CONTENT_HEIGHT)
		row = TERMINAL_CONTENT_HEIGHT - 1;
	return row + TERMINAL_CONTENT_START_ROW;
}

void terminal_scroll_up(void)
{
	struct terminal_screen* screen = terminal_current_screen();

	for (size_t y = TERMINAL_CONTENT_START_ROW + 1; y < VGA_HEIGHT; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			screen->cells[(y - 1) * VGA_WIDTH + x] = screen->cells[y * VGA_WIDTH + x];
			screen->is_tab_space[y - 1][x] = screen->is_tab_space[y][x];
			screen->is_cell_occupied[y - 1][x] = screen->is_cell_occupied[y][x];
		}
		screen->locked_prefix_col[y - 1] = screen->locked_prefix_col[y];
	}
	for (size_t x = 0; x < VGA_WIDTH; x++)
	{
		screen->cells[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', screen->color);
		screen->is_tab_space[VGA_HEIGHT - 1][x] = false;
		screen->is_cell_occupied[VGA_HEIGHT - 1][x] = false;
	}
	screen->locked_prefix_col[VGA_HEIGHT - 1] = 0;
}

static void terminal_write_text_for_screen(size_t screen_id, size_t row,
		size_t col, const char* text, uint8_t color)
{
	struct terminal_screen* screen = &screens[screen_id];
	size_t i = 0;

	while (text[i] && (col + i) < VGA_WIDTH && row < VGA_HEIGHT)
	{
		size_t x = col + i;
		size_t index = row * VGA_WIDTH + x;
		char c = text[i];

		screen->cells[index] = vga_entry(c, color);
		screen->is_tab_space[row][x] = false;
		screen->is_cell_occupied[row][x] = (c != ' ');
		i++;
	}
}

static void terminal_seed_initial_content_for_screen(size_t screen_id)
{
	struct terminal_screen* screen = &screens[screen_id];
	uint8_t text_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

	terminal_write_text_for_screen(screen_id, TERMINAL_CONTENT_START_ROW, 0,
		TERMINAL_BANNER_TEXT, text_color);
	terminal_write_text_for_screen(screen_id, TERMINAL_PROMPT_ROW, 0,
		TERMINAL_PROMPT_TEXT, text_color);
	screen->locked_prefix_col[TERMINAL_CONTENT_START_ROW] =
		(uint8_t)strlen(TERMINAL_BANNER_TEXT);
	screen->locked_prefix_col[TERMINAL_PROMPT_ROW] =
		(uint8_t)strlen(TERMINAL_PROMPT_TEXT);
	screen->row = TERMINAL_PROMPT_ROW;
	screen->column = TERMINAL_PROMPT_COL;
	screen->preferred_column = TERMINAL_PROMPT_COL;
}

static void terminal_draw_status_bar_for_screen(size_t screen_id)
{
	struct terminal_screen* screen = &screens[screen_id];
	uint8_t normal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
	uint8_t active_color = vga_entry_color(VGA_COLOR_DARK_GREY, VGA_COLOR_LIGHT_GREY);
	size_t x = 0;

	for (x = 0; x < VGA_WIDTH; x++)
	{
		screen->cells[TERMINAL_UI_ROW * VGA_WIDTH + x] = vga_entry(' ', normal_color);
		screen->is_tab_space[TERMINAL_UI_ROW][x] = false;
		screen->is_cell_occupied[TERMINAL_UI_ROW][x] = false;
	}
	x = 0;
	for (size_t i = 0; i < TERMINAL_SCREEN_COUNT; i++)
	{
		uint8_t box_color = (i == active_screen) ? active_color : normal_color;
		char label[9];

		if (x + 8 > VGA_WIDTH)
			break;
		label[0] = '[';
		label[1] = 'A';
		label[2] = 'l';
		label[3] = 't';
		label[4] = '+';
		label[5] = (char)('1' + i);
		label[6] = ']';
		label[7] = ' ';
		label[8] = '\0';
		for (size_t j = 0; j < 8; j++)
			screen->cells[TERMINAL_UI_ROW * VGA_WIDTH + x + j] = vga_entry(label[j], box_color);
		x += 8;
	}
}

void terminal_redraw_status_bars(void)
{
	for (size_t i = 0; i < TERMINAL_SCREEN_COUNT; i++)
		terminal_draw_status_bar_for_screen(i);
}

void terminal_initialize(void)
{
	for (size_t screen_id = 0; screen_id < TERMINAL_SCREEN_COUNT; screen_id++)
	{
		/* init history */
		screens[screen_id].history_count = 0;
		screens[screen_id].history_next = 0;
		screens[screen_id].history_nav = -1;
		screens[screen_id].history_temp[0] = '\0';
		for (size_t h = 0; h < 32; h++)
			screens[screen_id].history[h][0] = '\0';

		screens[screen_id].row = TERMINAL_PROMPT_ROW;
		screens[screen_id].column = TERMINAL_PROMPT_COL;
		screens[screen_id].preferred_column = TERMINAL_PROMPT_COL;
		screens[screen_id].color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
		screens[screen_id].ansi_bold = false;
		for (size_t y = 0; y < VGA_HEIGHT; y++)
		{
			for (size_t x = 0; x < VGA_WIDTH; x++)
			{
				const size_t index = y * VGA_WIDTH + x;
				screens[screen_id].cells[index] = vga_entry(' ', screens[screen_id].color);
				screens[screen_id].is_tab_space[y][x] = false;
				screens[screen_id].is_cell_occupied[y][x] = false;
			}
			screens[screen_id].locked_prefix_col[y] = 0;
		}
		terminal_seed_initial_content_for_screen(screen_id);
	}
	active_screen = 0;
	terminal_redraw_status_bars();
	terminal_flush_active_screen();
	terminal_update_cursor();
}

void terminal_clear(void)
{
	struct terminal_screen* screen = terminal_current_screen();

	for (size_t y = TERMINAL_CONTENT_START_ROW; y < VGA_HEIGHT; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			size_t index = y * VGA_WIDTH + x;
			screen->cells[index] = vga_entry(' ', screen->color);
			screen->is_tab_space[y][x] = false;
			screen->is_cell_occupied[y][x] = false;
		}
		screen->locked_prefix_col[y] = 0;
	}
	screen->row = TERMINAL_CONTENT_START_ROW;
	screen->column = 0;
	screen->preferred_column = 0;
	terminal_redraw_status_bars();
	terminal_flush_active_screen();
	terminal_update_cursor();
}

void terminal_reset_session(void)
{
	struct terminal_screen* screen = terminal_current_screen();

	for (size_t y = TERMINAL_CONTENT_START_ROW; y < VGA_HEIGHT; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			size_t index = y * VGA_WIDTH + x;
			screen->cells[index] = vga_entry(' ', screen->color);
			screen->is_tab_space[y][x] = false;
			screen->is_cell_occupied[y][x] = false;
		}
		screen->locked_prefix_col[y] = 0;
	}
	terminal_seed_initial_content_for_screen(active_screen);
	terminal_redraw_status_bars();
	terminal_flush_active_screen();
	terminal_update_cursor();
}
