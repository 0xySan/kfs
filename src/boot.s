; Declare constants for the multiboot header.
MB_ALIGN    equ (1 << 0)                  ; align loaded modules on page boundaries
MB_MEMINFO  equ (1 << 1)                  ; provide memory map
MB_FLAGS    equ (MB_ALIGN | MB_MEMINFO)   ; this is the Multiboot flag field
MB_MAGIC    equ 0x1BADB002                ; magic number lets bootloader find the header
MB_CHECKSUM equ -(MB_MAGIC + MB_FLAGS)    ; checksum of above, to prove we are multiboot

extern kernel_main

global _start

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

    ; Must contain the 32-bit physical address of the Multiboot information structure provided by the boot loader.
    ; This address is only valid after the boot loader has transferred control to the operating system, and is not valid before then.
    push ebx        ; arg2: multiboot info pointer

    ; Must contain the magic value ‘0x2BADB002’;
    ; the presence of this value indicates to the operating system that it was loaded by a Multiboot-compliant boot loader
    ; (e.g. as opposed to another type of boot loader that the operating system can also be loaded from).
    push eax        ; arg1: magic number

    call kernel_main

    ; Infinite loop to prevent the kernel from exiting.
    cli
.hang:
    hlt
    jmp .hang
