#include "../includes/kernel.h"

int execute_command(const char* command)
{
	char line[128];
	char* argv[8];
	size_t argc;

	if (!command)
		return 1;
	if (ft_strcmp(command, "\n") == 0)
		return 0;
	ft_strlcpy(line, command, sizeof(line));
	argc = split_words(line, argv, 8);
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
	{
		terminal_writestring("help\t -\tA help message to show you each commands available\n");
		terminal_writestring("clear\t -\tClear the terminal screen\n");
		terminal_writestring("echo\t -\tPrint the arguments passed to it\n");
		terminal_writestring("flags\t -\tPrint multiboot flags\n");
		terminal_writestring("dump\t -\tDump the kernel stack (usage: dump [number])\n");
		terminal_writestring("shutdown -\tPower off the machine\n");
		terminal_writestring("reboot\t -\tRestart the machine\n");
		terminal_writestring("halt\t -\tHalt the machine\n");
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
			for (size_t j = 0; argv[i][j]; j++)
			{
				if (argv[i][j] == '\\' && argv[i][j + 1])
				{
					j++;
					if (argv[i][j] == 'n')
						terminal_putchar('\n');
					else if (argv[i][j] == 't')
						terminal_putchar('\t');
					else if (argv[i][j] == 'b')
						terminal_putchar('\b');
					else if (argv[i][j] == '\\')
						terminal_putchar('\\');
					else
						terminal_putchar(argv[i][j]);
				}
				else
					terminal_putchar(argv[i][j]);
			}
			if (i + 1 < argc)
				terminal_putchar(' ');
		}
		terminal_write("\n", 1);
		return 0;
	}

	if (ft_strcmp(argv[0], "ls") == 0)
	{
		terminal_writestring("/\n");
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

	if (ft_strcmp(argv[0], "flags") == 0)
	{
		kernel_print_multiboot_flags();
		return 0;
	}

	if (ft_strcmp(argv[0], "halt") == 0)
	{
		kprintf("Halting forever and ever...\n");
		kernel_halt_forever();
	}

	if (ft_strcmp(argv[0], "^C") == 0)
		return 0;

	terminal_writestring("Unknown command. Type help\n");
	return 1;
}