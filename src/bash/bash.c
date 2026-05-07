/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bash.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:27:30 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:46:47 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

int execute_command(const char* command)
{
	char* argv[8] = {0};
	size_t argc;

	if (!command)
		return 1;
	if (ft_strcmp(command, "\n") == 0)
		return 0;
	argc = split_words((char *)command, argv, 8);
	if (argc == 0)
		return 0;

	if (ft_strcmp(argv[0], "bash$") == 0)
	{
		if (argc > 1)
		{
			for (size_t i = 0; i < argc - 1; i++)
				argv[i] = argv[i + 1];
			argc--;
		}
		else
			return 0;
	}

	if (ft_strcmp(argv[0], "help") == 0)
		return help();

    if (ft_strcmp(argv[0], "clear") == 0)
    {
        terminal_clear();
        return 0;
    }

	if (ft_strcmp(argv[0], "echo") == 0)
		return echo(argc, argv);

	if (ft_strcmp(argv[0], "ls") == 0)
	{
		terminal_writestring("/\n");
		return 0;
	}

	if (ft_strcmp(argv[0], "shutdown") == 0)
		return shutdown();

	if (ft_strcmp(argv[0], "reboot") == 0)
		return reboot();

	if (ft_strcmp(argv[0], "dump") == 0)
		return dump(argv);

	if (ft_strcmp(argv[0], "flags") == 0)
	{
		kernel_print_multiboot_flags();
		return 0;
	}

	if (ft_strcmp(argv[0], "halt") == 0)
		return halt();

	if (ft_strcmp(argv[0], "paging") == 0)
		return paging();

	if (ft_strcmp(argv[0], "free") == 0)
		return test_free(argv);

	if (ft_strcmp(argv[0], "malloc") == 0)
		return test_malloc(argv);

	if (ft_strcmp(argv[0], "panic") == 0)
		return panic(argv);

	if (ft_strcmp(argv[0], "fault") == 0)
		return fault();

	if (ft_strcmp(argv[0], "^C") == 0)
		return 0;

	printk("BASH", "Unknown command. Type help\n");
	return 1;
}