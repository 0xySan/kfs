; Declare constants for the multiboot header.
MB_ALIGN    equ (1 << 0)                  ; align loaded modules on page boundaries
MB_MEMINFO  equ (1 << 1)                  ; provide memory map
MB_FLAGS    equ (MB_ALIGN | MB_MEMINFO)   ; this is the Multiboot flag field
MB_MAGIC    equ 0x1BADB002                ; magic number lets bootloader find the header
MB_CHECKSUM equ -(MB_MAGIC + MB_FLAGS)    ; checksum of above, to prove we are multiboot

; Required location for runtime GDT.
GDT_BASE        equ 0x00000800
GDT_ENTRY_SIZE  equ 8
GDT_ENTRY_COUNT equ 7
GDT_TABLE_SIZE  equ GDT_ENTRY_SIZE * GDT_ENTRY_COUNT
GDT_DESC_ADDR   equ GDT_BASE + GDT_TABLE_SIZE

extern kernel_main
extern keyboard_handler

global _start
global load_idt
global keyboard_handler_stub

;
; Declare a multiboot header that marks the program as a kernel.
;
section .multiboot align=4
dd MB_MAGIC
dd MB_FLAGS
dd MB_CHECKSUM

;
; The multiboot standard does not define the value of esp and it is up to the
; kernel to provide a stack.
;
section .bss align=16
stack_bottom:
resb 16384                      ; 16 KiB
stack_top:

section .text
_start:
    ; Set up a valid aligned stack before jumping into C code.
    mov esp, stack_top

    ; Clear direction flag (required by System V ABI).
    cld

    ; Copy GDT + descriptor to the required physical address 0x00000800.
    mov esi, gdt_blob_start
    mov edi, GDT_BASE
    mov ecx, gdt_blob_end - gdt_blob_start
    rep movsb

    ; Load our GDT from 0x00000800.
    mov eax, GDT_DESC_ADDR
    lgdt [eax]
    jmp 0x08:reload_segments

reload_segments:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ax, 0x18
    mov ss, ax

    call kernel_main

    ; Infinite loop to prevent the kernel from exiting.
    cli
.hang:
    hlt
    jmp .hang

; Load the IDT with the address of the IDT descriptor.
load_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ret

; Stub for IRQ1 keyboard handler.
keyboard_handler_stub:
    pusha
    call keyboard_handler
    popa
    iretd

;
; Global Descriptor Table template.
; Runtime copy lives at 0x00000800 and includes:
;   - null
;   - kernel code
;   - kernel data
;   - kernel stack
;   - user code
;   - user data
;   - user stack
;
section .rodata align=8
gdt_blob_start:
    ; Null descriptor
    dd 0
    dd 0
    ; Kernel code segment: base=0, limit=4G, 32-bit, ring 0, exec+read
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A
    db 0xCF
    db 0x00
    ; Kernel data segment: base=0, limit=4G, 32-bit, ring 0, read+write
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00
    ; Kernel stack segment: base=0, limit=4G, 32-bit, ring 0, expand-down rw
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x96
    db 0xCF
    db 0x00
    ; User code segment: base=0, limit=4G, 32-bit, ring 3, exec+read
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0xFA
    db 0xCF
    db 0x00
    ; User data segment: base=0, limit=4G, 32-bit, ring 3, read+write
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0xF2
    db 0xCF
    db 0x00
    ; User stack segment: base=0, limit=4G, 32-bit, ring 3, expand-down rw
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0xF6
    db 0xCF
    db 0x00

    ; GDT descriptor, copied right after the table to 0x00000800 + 56.
gdt_descriptor_template:
    dw GDT_TABLE_SIZE - 1
    dd GDT_BASE
gdt_blob_end:
