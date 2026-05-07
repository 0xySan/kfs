/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   layout.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/08 00:21:20 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/08 00:21:20 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"

int layout_switcher(int argc, char **argv)
{
	if (argc < 2)
	{
		printk("LAYOUT", "Usage: layout <layout_name>\n");
		printk("LAYOUT", "Available layouts:\n");
		printk("LAYOUT", "  en - English QWERTY\n");
		printk("LAYOUT", "  fr - French AZERTY\n");
		return 0;
	}
	const char *layout = argv[1];
	if (ft_strcmp(layout, "en") == 0)
	{
		used_map = scancode_map;
		used_shift_map = shift_scancode_map;
		printk("LAYOUT", "Switched to English QWERTY layout\n");
	}
	else if (ft_strcmp(layout, "fr") == 0)
	{
		used_map = scancode_map_azerty;
		used_shift_map = shift_scancode_map_azerty;
		printk("LAYOUT", "Switched to French AZERTY layout\n");
	}
	else
	{
		printk("LAYOUT", "Unknown layout: %s\n", layout);
		printk("LAYOUT", "Available layouts:\n");
		printk("LAYOUT", "  en - English QWERTY\n");
		printk("LAYOUT", "  fr - French AZERTY\n");
	}
	return 0;
}