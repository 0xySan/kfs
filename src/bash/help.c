/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   help.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:27:51 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:50:44 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

int help(void)
{
	terminal_writestring("help\t -\tA help message to show you each commands available\n");
	terminal_writestring("clear\t -\tClear the terminal screen\n");
	terminal_writestring("echo\t -\tPrint the arguments passed to it (usage: echo [arguments])\n");
	terminal_writestring("flags\t -\tPrint multiboot flags\n");
	terminal_writestring("dump\t -\tDump the kernel stack (usage: dump [number])\n");
	terminal_writestring("shutdown -\tPower off the machine\n");
	terminal_writestring("reboot\t -\tRestart the machine\n");
	terminal_writestring("halt\t -\tHalt the machine\n");
	terminal_writestring("paging\t -\tShow whether paging is enabled\n");
	terminal_writestring("free\t -\tShow free memory frames (usage: free {virtual|frames})\n");
	terminal_writestring("panic\t - \tTrigger a kernel panic (usage: panic {warn|error})\n");
	terminal_writestring("fault\t - \tTrigger a page fault\n");
	return 0;
}