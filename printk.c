#include "kernel.h"

static void put_char_count(char c, int *written)
{
	terminal_write(&c, 1);
	(*written)++;
}

static void put_string_count(const char *str, int *written)
{
	size_t i;

	i = 0;
	while (str[i])
	{
		put_char_count(str[i], written);
		i++;
	}
}

static void put_unsigned_count(uintptr_t value, unsigned int base,
		int uppercase, int *written)
{
	char		buf[2 + sizeof(uintptr_t) * 8];
	const char	*digits;
	size_t		index;

	if (uppercase)
		digits = "0123456789ABCDEF";
	else
		digits = "0123456789abcdef";

	index = 0;
	if (value == 0)
		buf[index++] = '0';
	while (value > 0)
	{
		buf[index++] = digits[value % base];
		value /= base;
	}
	while (index > 0)
		put_char_count(buf[--index], written);
}

static void put_signed_count(int value, int *written)
{
	uintptr_t abs_value;

	if (value < 0)
	{
		put_char_count('-', written);
		abs_value = (uintptr_t)(-(value + 1)) + 1;
	}
	else
		abs_value = (uintptr_t)value;
	put_unsigned_count(abs_value, 10, 0, written);
}

static int vkprintf(const char *format, va_list args)
{
	int written;

	written = 0;
	while (*format)
	{
		if (*format != '%')
		{
			put_char_count(*format++, &written);
			continue;
		}
		format++;
		if (*format == '\0')
			break;
		if (*format == '%')
			put_char_count('%', &written);
		else if (*format == 'c')
			put_char_count((char)va_arg(args, int), &written);
		else if (*format == 's')
		{
			const char *str = va_arg(args, const char *);

			if (!str)
				str = "(null)";
			put_string_count(str, &written);
		}
		else if (*format == 'd' || *format == 'i')
			put_signed_count(va_arg(args, int), &written);
		else if (*format == 'u')
			put_unsigned_count((uintptr_t)va_arg(args, unsigned int), 10, 0, &written);
		else if (*format == 'x')
			put_unsigned_count((uintptr_t)va_arg(args, unsigned int), 16, 0, &written);
		else if (*format == 'X')
			put_unsigned_count((uintptr_t)va_arg(args, unsigned int), 16, 1, &written);
		else if (*format == 'p')
		{
			put_string_count("0x", &written);
			put_unsigned_count((uintptr_t)va_arg(args, void *), 16, 0, &written);
		}
		else
		{
			put_char_count('%', &written);
			put_char_count(*format, &written);
		}
		format++;
	}
	return written;
}

int kprintf(const char *format, ...)
{
	va_list args;
	int		written;

	va_start(args, format);
	written = vkprintf(format, args);
	va_end(args);
	return written;
}

int printk(const char *level, const char *format, ...)
{
	va_list args;
	int		written;

	written = 0;
	if (level)
	{
		put_char_count('[', &written);
		put_string_count(level, &written);
		put_string_count("] ", &written);
	}
	va_start(args, format);
	written += vkprintf(format, args);
	va_end(args);
	return written;
}
