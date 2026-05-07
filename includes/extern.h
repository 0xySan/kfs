/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   extern.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:29:50 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 23:53:51 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXTERN_H
# define EXTERN_H

#include "define.h"
#include "structs.h"

extern const char *colors[];

extern pde_t pd[1024];
extern pte_t pt[1024];

extern uint32_t kernel_end;

extern multiboot_info_t *g_mbi;

extern uint32_t	*bitmap;		// pointer to where bitmap lives in RAM
extern uint32_t	bitmap_size;	// number of uint32_t entries
extern uint32_t	mem_start;		// first managed frame address
extern uint32_t	total_frames;	// total frames being tracked

extern syscall_fn_t syscall_table[];

#endif // EXTERN_H