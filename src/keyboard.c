#include "../includes/kernel.h"

static int shift_held = 0;
static int caps_lock_on = 0;
static int alt_held = 0;
static int ctrl_held = 0;

static int handle_screen_shortcut(uint8_t scancode)
{
	if (scancode >= 0x02 && scancode < 0x02 + TERMINAL_SCREEN_COUNT)
	{
		terminal_switch_screen((size_t)(scancode - 0x02));
		return 1;
	}
	return 0;
}

char scancode_to_ascii(uint8_t scancode)
{
	if (shift_held) {
		if (scancode < 128)
			return shift_scancode_map[scancode];
		return 0;
	}
	if (scancode < 128)
		return scancode_map[scancode];
	return 0;
}

void keyboard_handler(void)
{
	uint8_t scancode = inb(0x60);
	if (scancode == 0x2A || scancode == 0x36)
		shift_held = 1;
	else if (scancode == 0xAA || scancode == 0xB6)
		shift_held = 0;
	if (scancode == 0x38)
		alt_held = 1;
	else if (scancode == 0xB8)
		alt_held = 0;
	if (scancode == 0x1D)
		ctrl_held = 1;
	else if (scancode == 0x9D)
		ctrl_held = 0;
	if (scancode == 0x3A)
		caps_lock_on = !caps_lock_on;
	if (scancode == 0x4D || scancode == 0x4B)
		handle_arrow_keys(scancode);
	if (!(scancode & 0x80)) {
		if (alt_held && handle_screen_shortcut(scancode))
		{
			outb(0x20, 0x20);
			return;
		}
		char c = scancode_to_ascii(scancode);
		if (ctrl_held)
		{
			if (scancode == 0x2E) // Ctrl + C
				terminal_write_input("^C\n", 3);
			else if (scancode == 0x20) // Ctrl + D
				terminal_reset_session();
		}
		else if (c)
		{
			if (caps_lock_on && c >= 'a' && c <= 'z')
				c -= 32;
			terminal_write_input(&c, 1);
		}
	}
	outb(0x20, 0x20);
}
