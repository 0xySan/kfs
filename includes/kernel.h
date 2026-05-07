/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kernel.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:29:39 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 01:38:47 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef KERNEL_H
# define KERNEL_H

# include "va_arg.h"
# include "define.h"
# include "helpers.h"
# include "structs.h"
# include "inline.h"
# include "keyboard.h"
# include "extern.h"
# include "../src/terminal/terminal.h"
# include "../src/memory/memory.h"

void load_idt(struct idt_ptr *ptr);
void gdt_init(void);
void keyboard_handler_stub(void);
void page_fault_handler_stub(void);
void page_fault_handler(uint32_t error_code);
void keyboard_handler(void);
void idt_set_gate(uint8_t num, uint32_t handler_address, uint16_t selector, uint8_t type_attr);
void idt_init(void);
void pic_init(void);
void handle_backspace(void);
int kprintf(const char *format, ...);
int printk(const char *level, const char *format, ...);
void kernel_shutdown(void);
void kernel_reboot(void);
void kernel_halt_forever(void);
void kernel_set_multiboot_info(multiboot_info_t *mbi);
void kernel_print_multiboot_flags(void);
void kshow_free_space(void);
void kwarn(const char *msg);
void kpanic(const char *msg);

#endif
