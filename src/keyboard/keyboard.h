/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   keyboard.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:29:35 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/08 00:35:17 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef KEYBOARD_H
# define KEYBOARD_H

#include "../../includes/structs.h"
#include "../../includes/inline.h"
#include "../terminal/terminal.h"

extern const unsigned char scancode_map[128];
extern const unsigned char shift_scancode_map[128];
extern const unsigned char scancode_map_azerty[128];
extern const unsigned char shift_scancode_map_azerty[128];

extern const unsigned char *used_map;
extern const unsigned char *used_shift_map;

void keyboard_handler(registers_t *_unused);
void handle_backspace(void);

#endif // KEYBOARD_H