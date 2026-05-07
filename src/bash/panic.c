/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   panic.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:28:23 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:28:24 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

int panic(char *argv[])
{
	if (!argv[1])
		printk("bash", "Usage: panic [warn|error]\n");
	else if (ft_strcmp(argv[1], "warn") == 0)
		kwarn("This is a warning message");
	else if (ft_strcmp(argv[1], "error") == 0)
		kpanic("This is a panic message");
	else
		printk("bash", "Usage: panic [warn|error]\n");
	return 0;
}