/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   echo.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:27:40 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:27:41 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

int echo(unsigned int argc, char *argv[])
{
	char	*buffer;
	size_t	capacity;
	size_t	pos;

	capacity = 1;
	for (size_t i = 1; i < argc; i++)
		capacity += strlen(argv[i]) + 1;
	buffer = (char *)kmalloc(capacity);
	if (!buffer)
		return 1;
	pos = 0;
	for (size_t i = 1; i < argc; i++)
	{
		for (size_t j = 0; argv[i][j]; j++)
		{
			if (argv[i][j] == '\\' && argv[i][j + 1])
			{
				j++;
				if (argv[i][j] == 'n')
					buffer[pos++] = '\n';
				else if (argv[i][j] == 't')
					buffer[pos++] = '\t';
				else if (argv[i][j] == 'b')
					buffer[pos++] = '\b';
				else if (argv[i][j] == 'e')
					buffer[pos++] = '\033';
				else if (argv[i][j] == '\\')
					buffer[pos++] = '\\';
				else
					buffer[pos++] = argv[i][j];
			}
			else
				buffer[pos++] = argv[i][j];
		}
		if (i + 1 < argc)
			buffer[pos++] = ' ';
	}
	buffer[pos++] = '\n';
	buffer[pos] = '\0';
	terminal_write(buffer, pos);
	kfree(buffer);
	return 0;
}