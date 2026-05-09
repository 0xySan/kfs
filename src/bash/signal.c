/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 06:00:00 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/09 05:31:09 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

static void signal_print_callback(uint32_t sig, void *payload)
{
    (void)sig;
    if (payload)
        printk("SIGNAL", "callback payload: %s\n", (const char *)payload);
    else
        printk("SIGNAL", "callback fired with no payload\n");
}

int signal_cmd(char *argv[])
{
    if (!argv[1])
    {
        printk("bash", "Usage: signal register <num> print | signal send <num> [message]\n");
        return 0;
    }

    if (ft_strcmp(argv[1], "register") == 0)
    {
        if (!argv[2] || !argv[3])
        {
            printk("bash", "Usage: signal register <num> print\n");
            return 0;
        }
        int num = atoi(argv[2]);
        if (ft_strcmp(argv[3], "print") == 0)
        {
            register_signal_callback((uint32_t)num, signal_print_callback);
            printk("bash", "registered print callback for signal %d\n", num);
            return 0;
        }
        printk("bash", "Unknown callback type. Supported: print\n");
        return 0;
    }

    if (ft_strcmp(argv[1], "send") == 0 || ft_strcmp(argv[1], "schedule") == 0)
    {
        if (!argv[2])
        {
            printk("bash", "Usage: signal send <num> [message]\n");
            return 0;
        }
        int num = atoi(argv[2]);
        void *payload = NULL;
        if (argv[3])
            payload = argv[3];
        schedule_signal((uint32_t)num, payload);
        printk("bash", "scheduled signal %d\n", num);
        return 0;
    }

    printk("bash", "Unknown signal subcommand\n");
    return 0;
}
