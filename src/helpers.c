#include "../includes/helpers.h"

int atoi(const char* str)
{
	int result = 0;
	int sign = 1;

	while (*str == ' ' || *str == '\t')
		str++;
	if (*str == '-')
	{
		sign = -1;
		str++;
	}
	else if (*str == '+')
	{
		str++;
	}
	while (*str >= '0' && *str <= '9')
	{
		result = result * 10 + (*str - '0');
		str++;
	}
	return sign * result;
}

int ft_strcmp(const char* a, const char* b)
{
	while (*a && *b && *a == *b)
	{
		a++;
		b++;
	}
	return (unsigned char)*a - (unsigned char)*b;
}

size_t ft_strlcpy(char* dst, const char* src, size_t dst_size)
{
	size_t i = 0;

	if (dst_size == 0)
		return 0;
	while (src[i] && i + 1 < dst_size)
	{
		dst[i] = src[i];
		i++;
	}
	dst[i] = '\0';
	return i;
}

size_t split_words(char* line, char** argv, size_t max_args)
{
	size_t argc = 0;
	char* p = line;

	while (*p && argc < max_args)
	{
		while (*p == ' ' || *p == '\t')
			p++;
		if (*p == '\0')
			break;
		argv[argc++] = p;
		while (*p && *p != ' ' && *p != '\t')
			p++;
		if (*p == '\0')
			break;
		*p = '\0';
		p++;
	}
	return argc;
}

size_t strlen(const char* str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}