/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   terminal_render.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:26:12 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:26:12 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "terminal.h"

// terminal_render.c - Character and element rendering

void terminal_render_char(char c)
{
	struct terminal_screen* screen = terminal_current_screen();

	if (screen->row < TERMINAL_CONTENT_START_ROW)
		screen->row = TERMINAL_CONTENT_START_ROW;
	size_t index = screen->row * VGA_WIDTH + screen->column;

	screen->cells[index] = vga_entry(c, screen->color);
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

void terminal_render_newline(void)
{
	struct terminal_screen* screen = terminal_current_screen();

	if (screen->row < TERMINAL_CONTENT_START_ROW)
		screen->row = TERMINAL_CONTENT_START_ROW;

	screen->column = 0;
	if (++screen->row == VGA_HEIGHT)
	{
		terminal_scroll_up();
		screen->row = VGA_HEIGHT - 1;
	}
}

void terminal_render_tab(void)
{
	struct terminal_screen* screen = terminal_current_screen();
	size_t tab_width = TAB_WIDTH - (screen->column % TAB_WIDTH);

	for (size_t i = 0; i < tab_width; i++)
		terminal_render_char(' ');
}

void terminal_render_backspace(void)
{
	struct terminal_screen* screen = terminal_current_screen();

	if (screen->row < TERMINAL_CONTENT_START_ROW)
	{
		screen->row = TERMINAL_CONTENT_START_ROW;
		return;
	}

	if (screen->column == 0)
	{
		if (screen->row <= TERMINAL_CONTENT_START_ROW)
			return;
		screen->row--;
		screen->column = VGA_WIDTH - 1;
	}
	else
		screen->column--;
	screen->cells[screen->row * VGA_WIDTH + screen->column] = vga_entry(' ', screen->color);
}

void terminal_putchar(char c)
{
	if (c == '\n')
		terminal_render_newline();
	else if (c == '\t')
		terminal_render_tab();
	else if (c == '\b')
		terminal_render_backspace();
	else
		terminal_render_char(c);
	terminal_flush_active_screen();
	terminal_update_cursor();
}
