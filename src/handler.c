/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handler.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 22:40:09 by etaquet           #+#    #+#             */
/*   Updated: 2026/05/07 22:40:09 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/kernel.h"

/*
 * 0x00 	0 		#DE 	Fault		No 		Divide Error 								DIV and IDIV instructions.
 * 0x01 	1 		#DB 	Trap		No 		Debug Exception 							Instruction, data, and I/O breakpoints; single-step; and others.
 * 0x02 	2 		NMI 	Interrupt	No 		NMI Interrupt 								Nonmaskable external interrupt.
 * 0x03 	3 		#BP 	Trap		No 		Breakpoint 									INT3 instruction.
 * 0x04 	4 		#OF 	Trap		No 		Overflow 									INTO instruction.
 * 0x05 	5 		#BR 	Fault		No 		BOUND Range Exceeded 						BOUND instruction.
 * 0x06 	6 		#UD 	Fault		No 		Invalid Opcode (Undefined Opcode) 			UD instruction or reserved opcode.
 * 0x07 	7 		#NM 	Fault		No 		Device Not Available (No Math Coprocessor) 	Floating-point or WAIT/FWAIT instruction.
 * 0x08 	8 		#DF 	Abort		Yes 	(zero) 	Double Fault 						Any instruction that can generate an exception, an NMI, or an INTR.
 * 0x09 	9 		N/A 	Fault		No 		Coprocessor Segment Overrun (reserved) 		Floating-point instruction.
 * 0x0A 	10 		#TS 	Fault		Yes 	Invalid TSS 								Task switch or TSS access.
 * 0x0B 	11 		#NP 	Fault		Yes 	Segment Not Present 						Loading segment registers or accessing system segments.
 * 0x0C 	12 		#SS 	Fault		Yes 	Stack-Segment Fault 						Stack operations and SS register loads.
 * 0x0D 	13 		#GP 	Fault		Yes 	General Protection Fault 					Any memory access or instruction fetch that violates protection rules, and other protection-related faults.
 * 0x0E 	14 		#PF 	Fault		Yes 	Page Fault 									Any memory access that references a page that is not present or violates page protections.
 * 0x0F 	15 		N/A 	Reserved 	No 		Reserved 									N/A
 * 0x10 	16 		#MF 	Fault		No 		FPU Floating-Point Error (Math Fault) 		Floating-point instructions.
 * 0x11 	17 		#AC 	Fault		Yes 	(zero) 	Alignment Check 					Any data reference in memory.
 * 0x12 	18 		#MC 	Abort		No 		Machine Check 								Error codes (if any) and source are model dependent.
 * 0x13 	19 		#XM 	Fault		No 		SIMD Floating-Point Exception 				SSE/SSE2/SSE3 floating-point instructions
 * 0x16
 * ⋮
 * 0x.. 	20-31 	N/A 	Reserved 	No 		Reserved 									N/A
 * ⋮
 * 0x..		32-255 	N/A 	Interrupt 	No 		N/A 										External interrupts. Software interrupts (INT n). System calls.
*/

static void divide_by_zero_handler(registers_t *regs) // INT 0
{
	kpanic("divide by zero", regs);
}

void debug_exception_handler(registers_t *regs) // INT 1
{
	printk("EXCEPTION", "Interrupt number: %u\n", regs->int_num);
}

void NMI_handler(registers_t *regs) // INT 2
{
	printk("EXCEPTION", "Non-Maskable Interrupt (NMI) occurred! Interrupt number: %u\n", regs->int_num);
}

void breakpoint_handler(registers_t *regs) // INT 3
{
	printk("EXCEPTION", "Breakpoint Exception occurred! Interrupt number: %u\n", regs->int_num);
}

void overflow_handler(registers_t *regs) // INT 4
{
	printk("EXCEPTION", "Overflow Exception occurred! Interrupt number: %u\n", regs->int_num);
}

void bounds_handler(registers_t *regs) // INT 5
{
	printk("EXCEPTION", "Bounds Check Exception occurred! Interrupt number: %u\n", regs->int_num);
}

static void invalid_opcode_handler(registers_t *regs) // INT 6
{
	kpanic("invalid opcode", regs);
}

void coprocessor_error_handler(registers_t *regs) // INT 7
{
	kpanic("Coprocessor Error", regs);
}

static void double_fault_handler(registers_t *regs) // INT 8
{
	printk("EXCEPTION", "Double Fault - error_code: 0x%x\n", regs->error_code);
	kpanic("double fault", regs);
}

void coprocessor_segment_overrun_handler(registers_t *regs) // INT 9

{
	kpanic("Coprocessor Segment Overrun", regs);
}

void invalid_task_state_handler(registers_t *regs) // INT 10
{
	printk("EXCEPTION", "Invalid TSS - error_code: 0x%x\n", regs->error_code);
	kpanic("Invalid Task State Segment (TSS)", regs);
}

void segment_not_present_handler(registers_t *regs) // INT 11
{
	printk("EXCEPTION", "Segment Not Present - error_code: 0x%x\n", regs->error_code);
	kpanic("Segment Not Present", regs);
}

void stack_fault_handler(registers_t *regs) // INT 12
{
	printk("EXCEPTION", "Stack Fault - error_code: 0x%x\n", regs->error_code);
	kpanic("Stack Fault", regs);
}

static void gpf_handler(registers_t *regs) // INT 13
{
	printk("EXCEPTION", "General Protection Fault - error_code: 0x%x\n", regs->error_code);
	kpanic("general protection fault", regs);
}

void page_fault_handler(registers_t *regs) // INT 14
{
	uint32_t faulting_addr;
	faulting_addr = read_cr2();

	uint32_t error_code = regs->error_code;
	// decode error code bits
	int present		= error_code & 0x1;	// 0 = not present, 1 = protection violation
	int rw			= error_code & 0x2;	// 0 = read, 1 = write
	int user		= error_code & 0x4;	// 0 = kernel, 1 = user mode
	int reserved	= error_code & 0x8;	// reserved bit overwrite

	printk("ERROR", "addr=0x%x error=0x%x\n", faulting_addr, error_code);

	if (!present)
		printk("ERROR", "cause: page not present\n");
	if (rw)
		printk("ERROR", "cause: write to read-only page\n");
	if (user)
		printk("ERROR", "cause: user mode access\n");
	if (reserved)
		printk("ERROR", "cause: reserved bits overwritten\n");

	if (user)
		kwarn("page fault in user space — would kill process");
	else
		kpanic("page fault in kernel space", regs);
}

void reserved_handler(registers_t *regs) // INT 15
{
	kpanic("Reserved Exception", regs);
}

void math_fault_handler(registers_t *regs) // INT 16
{
	kpanic("Math Fault", regs);
}

void alignment_check_handler(registers_t *regs) // INT 17
{
	printk("EXCEPTION", "Alignment Check - error_code: 0x%x\n", regs->error_code);
	kpanic("Alignment Check", regs);
}

void machine_check_handler(registers_t *regs) // INT 18
{
	kpanic("Machine Check", regs);
}

void simd_fault_handler(registers_t *regs) // INT 19
{
	kpanic("SIMD Floating-Point Exception", regs);
}

void register_exception_handlers(void)
{
	// Intel-defined exceptions
	register_isr_handler(0, divide_by_zero_handler);
	register_isr_handler(1, debug_exception_handler);
	register_isr_handler(2, NMI_handler);
	register_isr_handler(3, breakpoint_handler);
	register_isr_handler(4, overflow_handler);
	register_isr_handler(5, bounds_handler);
	register_isr_handler(6, invalid_opcode_handler);
	register_isr_handler(7, coprocessor_error_handler);
	register_isr_handler(8, double_fault_handler);
	register_isr_handler(9, coprocessor_segment_overrun_handler);
	register_isr_handler(10, invalid_task_state_handler);
	register_isr_handler(11, segment_not_present_handler);
	register_isr_handler(12, stack_fault_handler);
	register_isr_handler(13, gpf_handler);
	register_isr_handler(14, page_fault_handler);
	register_isr_handler(15, reserved_handler);
	register_isr_handler(16, math_fault_handler);
	register_isr_handler(17, alignment_check_handler);
	register_isr_handler(18, machine_check_handler);
	register_isr_handler(19, simd_fault_handler);
	// 20-31 are reserved by Intel
	// 32-255 are for user-defined IRQ handlers and software interrupts
	register_isr_handler(33, keyboard_handler); // IRQ1 - keyboard interrupt
}