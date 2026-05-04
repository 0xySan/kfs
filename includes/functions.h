#ifndef FUNCTIONS_H
# define FUNCTIONS_H

#include "define.h"
#include "structs.h"

void load_idt(struct idt_ptr *ptr);
void keyboard_handler_stub(void);
void page_fault_handler_stub(void);
void page_fault_handler(uint32_t error_code);
void keyboard_handler(void);
void idt_set_gate(uint8_t num, uint32_t handler_address, 
				  uint16_t selector, uint8_t type_attr);
void idt_init(void);
void pic_init(void);
void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_setrow(size_t row);
void terminal_setcolumn(size_t column);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_write_input(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void handle_backspace(void);
char scancode_to_ascii(uint8_t scancode);
int kprintf(const char *format, ...);
int printk(const char *level, const char *format, ...);
void dump_kernel_stack(size_t words);
void handle_arrow_keys(uint8_t arrow_key);
void terminal_switch_screen(size_t screen_index);
size_t terminal_get_active_screen(void);
int execute_command(const char* command);
void terminal_clear(void);
void terminal_reset_session(void);
void terminal_newline_with_prompt(void);
void kernel_shutdown(void);
void kernel_reboot(void);
void kernel_halt_forever(void);
void kernel_set_multiboot_info(multiboot_info_t *mbi);
void kernel_print_multiboot_flags(void);
void pfa_init(multiboot_info_t *mbi);
void *pfa_alloc_frame(void);
void pfa_free_frame(void *frame);
void show_free_frames(void);
void gdt_init(void);
void paging_init();
void pfa_mark_used(uintptr_t start, size_t size);
void vmm_map_page(uintptr_t virt, uintptr_t phys, uint32_t flags);
void vmm_unmap_page(uintptr_t virt);
uintptr_t vmm_get_physical(uintptr_t virt);
void kwarn(const char *msg);
void kpanic(const char *msg);

#endif