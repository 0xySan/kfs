#include "../includes/kernel.h"

uint32_t default_syscall_handler(registers_t *regs);

syscall_fn_t syscall_table[] = {
	[SYS_RESTART]	= default_syscall_handler,		// sys_restart_syscall
	[SYS_EXIT]		= default_syscall_handler,		// sys_exit
	[SYS_FORK]		= default_syscall_handler,		// sys_fork
	[SYS_READ]		= default_syscall_handler,		// sys_read
	[SYS_WRITE]		= default_syscall_handler,		// sys_write
	[SYS_OPEN]		= default_syscall_handler,		// sys_open
	[SYS_CLOSE]		= default_syscall_handler,		// sys_close
	// ...
};

void register_syscall_handler(uint32_t num, syscall_fn_t handler)
{
	if (num < SYSCALL_COUNT)
		syscall_table[num] = handler;
	else
		printk("SYSCALL", "Cannot register handler for syscall %u: out of bounds\n", num);
}

uint32_t default_syscall_handler(registers_t *regs)
{
	if (regs->eax < SYSCALL_COUNT)
		printk("SYSCALL", "No handler registered for syscall %u\n", regs->eax);
	else
		printk("SYSCALL", "Syscall number %u is out of bounds\n", regs->eax);
	if (regs->eax == SYS_EXIT)
		printk("SYSCALL", "sys_exit called with status %u\n", regs->ebx);
	regs->eax = (uint32_t)-1; // Return -1 to indicate error/unknown syscall
	return regs->eax;
}

void syscall_handler(registers_t *regs)
{
	uint32_t num = regs->eax;
	if (num >= SYSCALL_COUNT || !syscall_table[num])
	{
		default_syscall_handler(regs);
		return;
	}
	regs->eax = syscall_table[num](regs);
}