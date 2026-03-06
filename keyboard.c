#include "kernel.h"

static int shift_held = 0;
static int caps_lock_on = 0;

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
	if (scancode == 0x3A)
		caps_lock_on = !caps_lock_on;
	if (scancode == 0x4D || scancode == 0x4B)
		handle_arrow_keys(scancode);
	if (!(scancode & 0x80)) {
		char c = scancode_to_ascii(scancode);
		if (c)
		{
			if (caps_lock_on && c >= 'a' && c <= 'z')
				c -= 32;
			terminal_write(&c, 1);
		}
	}
	outb(0x20, 0x20);
}
