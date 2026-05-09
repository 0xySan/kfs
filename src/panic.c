/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   panic.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 01:29:11 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/09 05:24:57 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/kernel.h"

#define PANIC_STACK_WORDS 32

typedef struct s_panic_stack_snapshot
{
	uint32_t esp;
	uint32_t ebp;
	size_t word_count;
	uint32_t words[PANIC_STACK_WORDS];
	int valid;
} panic_stack_snapshot_t;

static panic_stack_snapshot_t g_panic_stack;

static void panic_read_current_stack(uint32_t *esp, uint32_t *ebp)
{
	__asm__ volatile ("mov %%esp, %0" : "=r"(*esp));
	__asm__ volatile ("mov %%ebp, %0" : "=r"(*ebp));
}

void panic_clear_registers(registers_t *regs)
{
	if (!regs)
		return;
	regs->edi = 0;
	regs->esi = 0;
	regs->ebp = 0;
	regs->esp = 0;
	regs->ebx = 0;
	regs->edx = 0;
	regs->ecx = 0;
	regs->eax = 0;
	regs->int_num = 0;
	regs->error_code = 0;
	regs->eip = 0;
	regs->cs = 0;
	regs->eflags = 0;
}

void panic_save_stack(registers_t *regs)
{
	uint32_t esp;
	uint32_t ebp;
	uint32_t *stack;
	uint32_t i;

	if (regs)
	{
		esp = regs->esp;
		ebp = regs->ebp;
	}
	else
		panic_read_current_stack(&esp, &ebp);
	stack = (uint32_t *)(uintptr_t)esp;
	g_panic_stack.esp = esp;
	g_panic_stack.ebp = ebp;
	g_panic_stack.word_count = PANIC_STACK_WORDS;
	g_panic_stack.valid = 1;
	for (i = 0; i < PANIC_STACK_WORDS; i++)
		g_panic_stack.words[i] = stack[i];
}

void panic_dump_stack(void)
{
	uint32_t i;

	if (!g_panic_stack.valid)
	{
		printk("PANIC", "no stack snapshot available\n");
		return;
	}
	printk("PANIC", "stack snapshot esp=0x%x ebp=0x%x\n",
		g_panic_stack.esp, g_panic_stack.ebp);
	for (i = 0; i < g_panic_stack.word_count; i++)
		printk("PANIC", "stack[%u]=0x%x\n", i, g_panic_stack.words[i]);
}

void kpanic(const char *msg, registers_t *regs)
{
	panic_save_stack(regs);
	printk("PANIC", "%s\n", msg);
	if (regs)
	{
		printk("PANIC", "eax=0x%x ebx=0x%x ecx=0x%x edx=0x%x\n",
			regs->eax, regs->ebx, regs->ecx, regs->edx);
		printk("PANIC", "eip=0x%x cs=0x%x eflags=0x%x\n",
			regs->eip, regs->cs, regs->eflags);
		panic_clear_registers(regs);
	}
	panic_dump_stack();
	__asm__ volatile ("cli"); // disable interrupts
	__asm__ volatile ("hlt"); // halt the CPU
}

void kwarn(const char *msg)
{
	printk("WARN", "%s\n", msg);
}