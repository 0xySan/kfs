/* Declare constants for the multiboot header. */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

/* 
Declare a multiboot header that marks the program as a kernel. These are magic
values that are documented in the multiboot standard. The bootloader will
search for this signature in the first 8 KiB of the kernel file, aligned at a
32-bit boundary. The signature is in its own section so the header can be
forced to be within the first 8 KiB of the kernel file.
*/
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/*
The multiboot standard does not define the value of the stack pointer register
(esp) and it is up to the kernel to provide a stack. This allocates room for a
small stack by creating a symbol at the bottom of it, then allocating 16384
bytes for it, and finally creating a symbol at the top. The stack grows
downwards on x86. The stack is in its own section so it can be marked nobits,
which means the kernel file is smaller because it does not contain an
uninitialized stack. The stack on x86 must be 16-byte aligned according to the
System V ABI standard and de-facto extensions. The compiler will assume the
stack is properly aligned and failure to align the stack will result in
undefined behavior.
*/
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

/*
The linker script specifies _start as the entry point to the kernel and the
bootloader will jump to this position once the kernel has been loaded. It
doesn't make sense to return from this function as the bootloader is gone.
*/
.section .text
.global _start
.type _start, @function
_start:
	/*
	The bootloader has loaded us into 32-bit protected mode on a x86
	machine. Interrupts are disabled. Paging is disabled. The processor
	state is as defined in the multiboot standard. The kernel has full
	control of the CPU. The kernel can only make use of hardware features
	and any code it provides as part of itself. There's no printf
	function, unless the kernel provides its own <stdio.h> header and a
	printf implementation. There are no security restrictions, no
	safeguards, no debugging mechanisms, only what the kernel provides
	itself. It has absolute and complete power over the
	machine.
	*/

	/*
	To set up a stack, we set the esp register to point to the top of the
	stack (as it grows downwards on x86 systems). This is necessarily done
	in assembly as languages such as C cannot function without a stack.
	*/
	mov $stack_top, %esp

	/* Clear direction flag (required by System V ABI) */
	cld

	/* Copy GDT + descriptor to the required physical address 0x00000800 */
	mov $gdt_blob_start, %esi
	mov $GDT_BASE, %edi
	mov $(gdt_blob_end - gdt_blob_start), %ecx
	rep movsb

	/* Load our GDT from 0x00000800 — GRUB's GDT is not guaranteed to persist */
	mov $GDT_DESC_ADDR, %eax
	lgdt (%eax)
	ljmp $0x08, $reload_segments

reload_segments:
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov $0x18, %ax
	mov %ax, %ss

	call kernel_main

	/*
	If the system has nothing more to do, put the computer into an
	infinite loop. To do that:
	1) Disable interrupts with cli (clear interrupt enable in eflags).
	   They are already disabled by the bootloader, so this is not needed.
	   Mind that you might later enable interrupts and return from
	   kernel_main (which is sort of nonsensical to do).
	2) Wait for the next interrupt to arrive with hlt (halt instruction).
	   Since they are disabled, this will lock up the computer.
	3) Jump to the hlt instruction if it ever wakes up due to a
	   non-maskable interrupt occurring or due to system management mode.
	*/
	cli

/* Infinite loop to prevent the kernel from exiting. */
1:	hlt
	jmp 1b

/*
Set the size of the _start symbol to the current location '.' minus its start.
This is useful when debugging or when you implement call tracing.
*/
.size _start, . - _start

/* Load the IDT with the address of the IDT descriptor. The IDT is set up in
kernel.c and the descriptor is defined in kernel.h. */
.global load_idt
load_idt:
    mov 4(%esp), %eax
    lidt (%eax)
    ret

/* This is a stub for the keyboard interrupt handler. The actual handler is
defined in keyboard.c. The keyboard interrupt is IRQ1, which corresponds to
interrupt vector 33 (0x21) in the IDT. The handler must end with an iret
instruction to return from the interrupt. */
.global keyboard_handler_stub
keyboard_handler_stub:
    pusha
    call keyboard_handler
    popa
    iret

/* Required location for runtime GDT. */
.set GDT_BASE, 0x00000800
.set GDT_ENTRY_SIZE, 8
.set GDT_ENTRY_COUNT, 7
.set GDT_TABLE_SIZE, GDT_ENTRY_SIZE * GDT_ENTRY_COUNT
.set GDT_DESC_ADDR, GDT_BASE + GDT_TABLE_SIZE

/*
Global Descriptor Table template.
Runtime copy lives at 0x00000800 and includes:
  - null
  - kernel code
  - kernel data
  - kernel stack
  - user code
  - user data
  - user stack
*/
.section .rodata
.align 8
gdt_blob_start:
	/* Null descriptor */
	.long 0
	.long 0
	/* Kernel code segment: base=0, limit=4G, 32-bit, ring 0, exec+read */
	.word 0xFFFF      /* Limit bits 0–15  = 0xFFFF                      */
	.word 0x0000      /* Base  bits 0–15  = 0x0000                      */
	.byte 0x00        /* Base  bits 16–23 = 0x00                        */
	.byte 0x9A        /* Access byte                                    */
	.byte 0xCF        /* Flags (4 bits) + Limit bits 16–19 (4 bits)     */
	.byte 0x00        /* Base  bits 24–31 = 0x00                        */
	/* Kernel data segment: base=0, limit=4G, 32-bit, ring 0, read+write */
	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.byte 0x92
	.byte 0xCF
	.byte 0x00
	/* Kernel stack segment: base=0, limit=4G, 32-bit, ring 0, expand-down rw */
	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.byte 0x96
	.byte 0xCF
	.byte 0x00
	/* User code segment: base=0, limit=4G, 32-bit, ring 3, exec+read */
	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.byte 0xFA
	.byte 0xCF
	.byte 0x00
	/* User data segment: base=0, limit=4G, 32-bit, ring 3, read+write */
	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.byte 0xF2
	.byte 0xCF
	.byte 0x00
	/* User stack segment: base=0, limit=4G, 32-bit, ring 3, expand-down rw */
	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.byte 0xF6
	.byte 0xCF
	.byte 0x00

/* GDT descriptor, copied right after the table to 0x00000800 + 56. */
gdt_descriptor_template:
	.word GDT_TABLE_SIZE - 1
	.long GDT_BASE
gdt_blob_end:
