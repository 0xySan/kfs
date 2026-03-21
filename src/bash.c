#include "../includes/kernel.h"

int execute_command(const char* command)
{
	char line[128];
	char* argv[8];
	size_t argc;

	if (!command)
		return 1;
	ft_strlcpy(line, command, sizeof(line));
	argc = split_words(line, argv, 8);
	if (argc == 0)
		return 0;

	if (ft_strcmp(argv[0], "help") == 0)
	{
		terminal_writestring("help - A help message to show you each commands available\n");
		terminal_writestring("clear - Clear the terminal screen\n");
		terminal_writestring("echo - Print the arguments passed to it\n");
		terminal_writestring("dump - Dump the kernel stack (usage: dump [number])\n");
		terminal_writestring("shutdown - Power off the machine\n");
		terminal_writestring("reboot - Restart the machine\n");
		return 0;
	}

    if (ft_strcmp(argv[0], "clear") == 0)
    {
        terminal_clear();
        return 0;
    }

	if (ft_strcmp(argv[0], "echo") == 0)
	{
		for (size_t i = 1; i < argc; i++)
		{
			terminal_writestring(argv[i]);
			if (i + 1 < argc)
				terminal_putchar(' ');
		}
		terminal_write("\n", 1);
		return 0;
	}

	if (ft_strcmp(argv[0], "shutdown") == 0)
	{
		terminal_writestring("Shutting down...\n");
		kernel_shutdown();
		return 0;
	}

	if (ft_strcmp(argv[0], "reboot") == 0)
	{
		terminal_writestring("Rebooting...\n");
		kernel_reboot();
		return 0;
	}

	if (ft_strcmp(argv[0], "dump") == 0)
	{
		if (argv[1])
		{
			size_t words = (size_t)atoi(argv[1]);
			if (words > 0)
				dump_kernel_stack(words);
			else
				terminal_writestring("Usage: dump [words]\n");
		}
		else
			dump_kernel_stack(8);
		return 0;
	}

	if (ft_strcmp(argv[0], "^C") == 0)
		return 0;

	terminal_writestring("Unknown command. Type help\n");
	return 1;
}