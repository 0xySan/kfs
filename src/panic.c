#include "../includes/kernel.h"

void kpanic(const char *msg)
{
    __asm__ volatile ("cli"); // disable interrupts to prevent further issues
    printk("PANIC", "%s\n", msg);
    kernel_halt_forever();
}

void kwarn(const char *msg)
{
    printk("WARN", "%s\n", msg);
}