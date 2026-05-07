/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   terminal.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:30:08 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:34:03 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TERMINAL_H
# define TERMINAL_H

#include "../../includes/structs.h"
#include "../../includes/inline.h"
#include "../../includes/helpers.h"

// Screen management
struct terminal_screen* terminal_current_screen(void);
void terminal_flush_active_screen(void);
void terminal_scroll_up(void);
void terminal_switch_screen(size_t screen_index);
size_t terminal_get_active_screen(void);
void terminal_redraw_status_bars(void);
void terminal_initialize(void);
void terminal_clear(void);
void terminal_reset_session(void);

// Cursor operations
void terminal_update_cursor(void);
void terminal_setrow(size_t row);
void terminal_setcolumn(size_t column);
void terminal_setcolor(uint8_t color);
void terminal_advance_cursor(void);
size_t terminal_cursor_index(void);
size_t terminal_clamp_content_row(size_t row);
size_t terminal_locked_prefix_col(size_t row);
int terminal_row_last_filled_col(size_t row);
size_t terminal_row_max_cursor_col(size_t row);
size_t terminal_tab_span_before_cursor(size_t row, size_t column);
size_t terminal_tab_span_after_cursor(size_t row, size_t column, size_t max_col);
void handle_arrow_keys(uint8_t arrow_key);

// Rendering
void terminal_render_char(char c);
void terminal_render_newline(void);
void terminal_render_tab(void);
void terminal_render_backspace(void);
void terminal_putchar(char c);

// Color/ANSI
void terminal_handle_escape_sequence(const char *data, size_t *i);

// Input
void terminal_insert_char(char c);
void terminal_delete_char_before_cursor(void);
void terminal_collect_current_command(char* out, size_t out_size);
void handle_backspace(void);
void terminal_newline_with_prompt(void);
void terminal_write_input(const char* data, size_t size);
int execute_command(const char* command);

// Output
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);

#endif
