/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   printk.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:29:14 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:53:08 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/kernel.h"

/* Internal print buffer - avoids flushing VGA for every single character.
 * terminal_write() already batches the flush at the end, so we just need
 * to collect all chars and call it once per printk/kprintf invocation. */
#define PRINTK_BUF_SIZE 512

typedef struct {
	char	buf[PRINTK_BUF_SIZE];
	size_t	len;
} t_printk_buf;

static void flush_buf(t_printk_buf *b)
{
	if (b->len > 0)
	{
		terminal_write(b->buf, b->len);
		b->len = 0;
	}
}

static void put_char_buf(char c, t_printk_buf *b)
{
	b->buf[b->len++] = c;
	if (b->len == PRINTK_BUF_SIZE)
		flush_buf(b);
}

static void put_string_buf(const char *str, t_printk_buf *b)
{
	size_t i = 0;
	while (str[i])
		put_char_buf(str[i++], b);
}

static void put_unsigned_buf(uintptr_t value, unsigned int base,
		int uppercase, t_printk_buf *b)
{
	char		tmp[2 + sizeof(uintptr_t) * 8];
	const char	*digits;
	size_t		index;

	digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
	index = 0;
	if (value == 0)
		tmp[index++] = '0';
	while (value > 0)
	{
		tmp[index++] = digits[value % base];
		value /= base;
	}
	while (index > 0)
		put_char_buf(tmp[--index], b);
}

static void put_signed_buf(int value, t_printk_buf *b)
{
	uintptr_t abs_value;

	if (value < 0)
	{
		put_char_buf('-', b);
		abs_value = (uintptr_t)(-(value + 1)) + 1;
	}
	else
		abs_value = (uintptr_t)value;
	put_unsigned_buf(abs_value, 10, 0, b);
}

static int vkprintf_buf(const char *format, va_list args, t_printk_buf *b)
{
	int written = 0;

	while (*format)
	{
		if (*format != '%')
		{
			put_char_buf(*format++, b);
			written++;
			continue;
		}
		format++;
		if (*format == '\0')
			break;
		if (*format == '%')
			{ put_char_buf('%', b); written++; }
		else if (*format == 'c')
			{ put_char_buf((char)va_arg(args, int), b); written++; }
		else if (*format == 's')
		{
			const char *str = va_arg(args, const char *);
			if (!str) str = "(null)";
			put_string_buf(str, b);
			written += (int)strlen(str);
		}
		else if (*format == 'd' || *format == 'i')
			put_signed_buf(va_arg(args, int), b);
		else if (*format == 'u')
			put_unsigned_buf((uintptr_t)va_arg(args, unsigned int), 10, 0, b);
		else if (*format == 'x')
			put_unsigned_buf((uintptr_t)va_arg(args, unsigned int), 16, 0, b);
		else if (*format == 'X')
			put_unsigned_buf((uintptr_t)va_arg(args, unsigned int), 16, 1, b);
		else if (*format == 'p')
		{
			put_string_buf("0x", b);
			put_unsigned_buf((uintptr_t)va_arg(args, void *), 16, 0, b);
		}
		else
		{
			put_char_buf('%', b);
			put_char_buf(*format, b);
		}
		format++;
	}
	return written;
}

int kprintf(const char *format, ...)
{
	t_printk_buf	b;
	va_list			args;
	int				written;

	b.len = 0;
	va_start(args, format);
	written = vkprintf_buf(format, args, &b);
	va_end(args);
	flush_buf(&b);   /* single terminal_write → single VGA flush */
	return written;
}

int printk(const char *level, const char *format, ...)
{
	t_printk_buf	b;
	va_list			args;
	int				written;

	b.len = 0;
	written = 0;
	if (level)
	{
		if (ft_strcmp(level, "PANIC") == 0 || ft_strcmp(level, "ERROR") == 0)
			put_string_buf(RED, &b);
		else if (ft_strcmp(level, "WARN") == 0)
			put_string_buf(YELLOW, &b);
		else
			put_string_buf(BLUE, &b);
		put_char_buf('[', &b);
		put_string_buf(level, &b);
		put_string_buf("]", &b);
		put_string_buf(RESET, &b);
		put_char_buf(' ', &b);
	}
	va_start(args, format);
	written += vkprintf_buf(format, args, &b);
	va_end(args);
	flush_buf(&b);   /* single terminal_write → single VGA flush */
	return written;
}