/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   terminal_color.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:25:25 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:25:26 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "terminal.h"

// terminal_color.c - ANSI color codes and SGR (Select Graphic Rendition) handling

static uint8_t terminal_ansi_fg_color(int code, bool bold)
{
	if (code >= 90 && code <= 97)
		return (uint8_t)(8 + (code - 90));
	if (code >= 30 && code <= 37)
	{
		uint8_t fg = (uint8_t)(code - 30);

		if (bold)
			fg = (uint8_t)(fg + 8);
		return fg;
	}
	return VGA_COLOR_LIGHT_GREY;
}

static uint8_t terminal_ansi_bg_color(int code)
{
	if (code >= 100 && code <= 107)
		return (uint8_t)(8 + (code - 100));
	if (code >= 40 && code <= 47)
		return (uint8_t)(code - 40);
	return VGA_COLOR_BLACK;
}

static void terminal_apply_sgr_sequence(const int *codes, size_t count)
{
	struct terminal_screen* screen = terminal_current_screen();
	uint8_t fg = (uint8_t)(screen->color & 0x0F);
	uint8_t bg = (uint8_t)((screen->color >> 4) & 0x0F);
	bool bold = screen->ansi_bold;
	bool fg_changed = false;

	if (count == 0)
	{
		fg = VGA_COLOR_LIGHT_GREY;
		bg = VGA_COLOR_BLACK;
		bold = false;
	}
	for (size_t i = 0; i < count; i++)
	{
		int code = codes[i];

		if (code == 0)
		{
			fg = VGA_COLOR_LIGHT_GREY;
			bg = VGA_COLOR_BLACK;
			bold = false;
			fg_changed = true;
		}
		else if (code == 1)
			bold = true;
		else if (code == 22)
			bold = false;
		else if ((code >= 30 && code <= 37) || (code >= 90 && code <= 97))
		{
			fg = terminal_ansi_fg_color(code, bold);
			fg_changed = true;
		}
		else if (code == 39)
		{
			fg = VGA_COLOR_LIGHT_GREY;
			fg_changed = true;
		}
		else if ((code >= 40 && code <= 47) || (code >= 100 && code <= 107))
			bg = terminal_ansi_bg_color(code);
		else if (code == 49)
			bg = VGA_COLOR_BLACK;
	}
	if (bold && !fg_changed && fg < VGA_COLOR_DARK_GREY)
		fg = (uint8_t)(fg + 8);
	screen->ansi_bold = bold;
	screen->color = vga_entry_color((enum vga_color)fg, (enum vga_color)bg);
}

void terminal_handle_escape_sequence(const char *data, size_t *i)
{
	int codes[8];
	size_t count = 0;
	int current = -1;
	size_t j = *i + 2;

	if (data[*i] != '\033' || data[*i + 1] != '[')
		return;
	while (data[j] && data[j] != 'm')
	{
		if (data[j] >= '0' && data[j] <= '9')
		{
			if (current < 0)
				current = 0;
			current = (current * 10) + (data[j] - '0');
		}
		else if (data[j] == ';')
		{
			if (count < 8)
				codes[count++] = (current < 0) ? 0 : current;
			current = -1;
		}
		else
			break;
		j++;
	}
	if (data[j] != 'm')
		return;
	if (current >= 0 && count < 8)
		codes[count++] = current;
	if (count == 0)
		codes[count++] = 0;
	terminal_apply_sgr_sequence(codes, count);
	*i = j;
}
