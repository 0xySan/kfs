#include "../includes/kernel.h"

static volatile uint16_t* const vga_buffer = (uint16_t*)VGA_MEMORY;
static struct terminal_screen screens[TERMINAL_SCREEN_COUNT];
static size_t active_screen;
static struct terminal_screen* terminal_current_screen(void);
static bool execute_on_newline = true;
static int terminal_row_last_filled_col(size_t row);
static void terminal_reset_line(size_t row);

void terminal_set_execute_on_newline(bool enabled)
{
	struct terminal_screen* screen = terminal_current_screen();
	
	if (!enabled)
	{
		terminal_reset_line(screen->row);
		screen->column = 0;
	}
	execute_on_newline = enabled;
}

static size_t terminal_locked_prefix_col(size_t row)
{
	return terminal_current_screen()->locked_prefix_col[row];
}

static void terminal_reset_line(size_t row)
{
	struct terminal_screen* screen = terminal_current_screen();

	for (size_t x = 0; x < VGA_WIDTH; x++)
	{
		screen->cells[row * VGA_WIDTH + x] = vga_entry(' ', screen->color);
		screen->is_tab_space[row][x] = false;
		screen->is_cell_occupied[row][x] = false;
	}
	screen->locked_prefix_col[row] = 0;
}

static void terminal_collect_current_command(char* out, size_t out_size)
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

static struct terminal_screen* terminal_current_screen(void)
{
	return &screens[active_screen];
}

static void terminal_insert_char(char c);

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

static size_t terminal_clamp_content_row(size_t row)
{
	if (row >= TERMINAL_CONTENT_HEIGHT)
		row = TERMINAL_CONTENT_HEIGHT - 1;
	return row + TERMINAL_CONTENT_START_ROW;
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

static void terminal_redraw_status_bars(void)
{
	for (size_t i = 0; i < TERMINAL_SCREEN_COUNT; i++)
		terminal_draw_status_bar_for_screen(i);
}

void terminal_flush_active_screen(void)
{
	struct terminal_screen* screen = terminal_current_screen();

	for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++)
		vga_buffer[i] = screen->cells[i];
}

static void terminal_update_cursor(void)
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

static size_t terminal_cursor_index(void)
{
	struct terminal_screen* screen = terminal_current_screen();

	return screen->row * VGA_WIDTH + screen->column;
}

static void terminal_scroll_up(void)
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

void terminal_newline_with_prompt(void)
{
	struct terminal_screen* screen = terminal_current_screen();
	char command[128];

	if (execute_on_newline)
		terminal_collect_current_command(command, sizeof(command));

	screen->column = 0;
	if (++screen->row == VGA_HEIGHT)
	{
		terminal_scroll_up();
		screen->row = VGA_HEIGHT - 1;
	}
	screen->locked_prefix_col[screen->row] = 0;

	if (execute_on_newline)
	{
		execute_on_newline = false;
		execute_command(command);
		execute_on_newline = true;
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
	}

	if (!execute_on_newline)
		return;
	screen->locked_prefix_col[screen->row] = (uint8_t)strlen(TERMINAL_PROMPT_TEXT);
	for (size_t i = 0; TERMINAL_PROMPT_TEXT[i] != '\0'; i++)
		terminal_insert_char(TERMINAL_PROMPT_TEXT[i]);
}

static void terminal_advance_cursor(void)
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

static void terminal_insert_char(char c)
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


static int terminal_row_last_filled_col(size_t row)
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

static size_t terminal_tab_span_before_cursor(size_t row, size_t column)
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

static size_t terminal_tab_span_after_cursor(size_t row, size_t column, size_t max_col)
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

void terminal_initialize(void)
{
	for (size_t screen_id = 0; screen_id < TERMINAL_SCREEN_COUNT; screen_id++)
	{
		screens[screen_id].row = TERMINAL_PROMPT_ROW;
		screens[screen_id].column = TERMINAL_PROMPT_COL;
		screens[screen_id].preferred_column = TERMINAL_PROMPT_COL;
		screens[screen_id].color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
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

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	struct terminal_screen* screen = terminal_current_screen();
	const size_t index = y * VGA_WIDTH + x;

	screen->cells[index] = vga_entry(c, color);
	screen->is_tab_space[y][x] = false;
	screen->is_cell_occupied[y][x] = (c != ' ');
	vga_buffer[index] = screen->cells[index];
}

void terminal_putchar(char c)
{
	terminal_insert_char(c);
	terminal_flush_active_screen();
	terminal_update_cursor();
}

void handle_backspace(void)
{
	struct terminal_screen* screen = terminal_current_screen();

	terminal_delete_char_before_cursor();
	screen->preferred_column = screen->column;
	terminal_flush_active_screen();
	terminal_update_cursor();
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

void terminal_write(const char* data, size_t size)
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

void terminal_writestring(const char* data)
{
	terminal_write(data, strlen(data));
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

size_t terminal_get_active_screen(void)
{
	return active_screen;
}
