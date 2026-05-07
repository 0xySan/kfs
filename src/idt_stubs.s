section .text

global load_idt
; Load the IDT with the address of the IDT descriptor.
load_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ret

%macro ISR_NOERR 1
global isr_stub_%1
isr_stub_%1:
    push dword 0
    push dword %1
    jmp isr_common
%endmacro

%macro ISR_ERR 1
global isr_stub_%1
isr_stub_%1:
    push dword %1
    jmp isr_common
%endmacro

ISR_NOERR 0   ; Division by Zero
ISR_NOERR 1   ; Debugger
ISR_NOERR 2   ; NMI
ISR_NOERR 3   ; Breakpoint
ISR_NOERR 4   ; Overflow
ISR_NOERR 5   ; Bounds
ISR_NOERR 6   ; Invalid Opcode
ISR_NOERR 7   ; Coprocessor not available
ISR_ERR   8   ; Double fault          ← error code (always 0)
ISR_NOERR 9   ; Coprocessor Segment Overrun
ISR_ERR   10  ; Invalid TSS           ← error code
ISR_ERR   11  ; Segment not present   ← error code
ISR_ERR   12  ; Stack Fault           ← error code
ISR_ERR   13  ; General protection    ← error code
ISR_ERR   14  ; Page fault            ← error code (you already have this)
ISR_NOERR 15  ; Reserved
ISR_NOERR 16  ; Math Fault
ISR_ERR   17  ; Alignment Check       ← error code
ISR_NOERR 18  ; Machine Check
ISR_NOERR 19  ; SIMD Floating-Point

; 20-31: reserved, all NOERR
%assign i 20
%rep 12
    ISR_NOERR i
%assign i i+1
%endrep

; 32-255 hardware IRQs
%assign i 32
%rep 224
    ISR_NOERR i
%assign i i+1
%endrep

extern generic_isr_handler

isr_common:
    pusha
    push esp              ; pointer to registers_t on stack
    call generic_isr_handler
    add esp, 4
    popa
    add esp, 8            ; clean int_num + error_code
    iretd

global isr_stub_table
isr_stub_table:
%assign i 0
%rep 256
    dd isr_stub_%+i
%assign i i+1
%endrep