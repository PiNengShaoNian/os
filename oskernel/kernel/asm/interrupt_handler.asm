[bits 32]
[SECTION .text]

extern printk
extern keymap_handler
extern exception_handler

global interrupt_handler_entry
interrupt_handler_entry:
    push msg
    call printk
    add esp, 4

    iret

; 键盘中断
global keymap_handler_entry
keymap_handler_entry:
    push 0x21
    call keymap_handler
    add esp, 4

    iret

; 异常处理
;
; idt_index
; edi
; esi
; ebp
; esp
; ebx
; edx
; ecx
; eax
; eip
; cs
; eflags
%macro INTERRUPT_HANDLER 1
global interrupt_handler_%1
interrupt_handler_%1:
    pushad

    push %1
    call exception_handler
    add esp, 4

    popad

    iret
%endmacro

INTERRUPT_HANDLER 0x00; divide by zero
INTERRUPT_HANDLER 0x01; debug
INTERRUPT_HANDLER 0x02; non maskable interrupt
INTERRUPT_HANDLER 0x03; breakpoint

INTERRUPT_HANDLER 0x04; overflow
INTERRUPT_HANDLER 0x05; bound range exceeded
INTERRUPT_HANDLER 0x06; invalid opcode
INTERRUPT_HANDLER 0x07; device not avilable

INTERRUPT_HANDLER 0x08; double fault
INTERRUPT_HANDLER 0x09; coprocessor segment overrun
INTERRUPT_HANDLER 0x0a; invalid TSS
INTERRUPT_HANDLER 0x0b; segment not present

INTERRUPT_HANDLER 0x0c; stack segment fault
INTERRUPT_HANDLER 0x0d; general protection fault
INTERRUPT_HANDLER 0x0e; page fault
INTERRUPT_HANDLER 0x0f; reserved

INTERRUPT_HANDLER 0x10; x87 floating point exception
INTERRUPT_HANDLER 0x11; alignment check
INTERRUPT_HANDLER 0x12; machine check
INTERRUPT_HANDLER 0x13; SIMD Floating - Point Exception

INTERRUPT_HANDLER 0x14; Virtualization Exception
INTERRUPT_HANDLER 0x15; Control Protection Exception

INTERRUPT_HANDLER 0x16; reserved
INTERRUPT_HANDLER 0x17; reserved
INTERRUPT_HANDLER 0x18; reserved
INTERRUPT_HANDLER 0x19; reserved
INTERRUPT_HANDLER 0x1a; reserved
INTERRUPT_HANDLER 0x1b; reserved
INTERRUPT_HANDLER 0x1c; reserved
INTERRUPT_HANDLER 0x1d; reserved
INTERRUPT_HANDLER 0x1e; reserved
INTERRUPT_HANDLER 0x1f; reserved

INTERRUPT_HANDLER 0x20; clock 时钟中断
INTERRUPT_HANDLER 0x21; 键盘中断
INTERRUPT_HANDLER 0x22
INTERRUPT_HANDLER 0x23
INTERRUPT_HANDLER 0x24
INTERRUPT_HANDLER 0x25
INTERRUPT_HANDLER 0x26
INTERRUPT_HANDLER 0x27
INTERRUPT_HANDLER 0x28; rtc 实时时钟
INTERRUPT_HANDLER 0x29
INTERRUPT_HANDLER 0x2a
INTERRUPT_HANDLER 0x2b
INTERRUPT_HANDLER 0x2c
INTERRUPT_HANDLER 0x2d
INTERRUPT_HANDLER 0x2e
INTERRUPT_HANDLER 0x2f

global interrupt_handler_table
interrupt_handler_table:
    dd interrupt_handler_0x00
    dd interrupt_handler_0x01
    dd interrupt_handler_0x02
    dd interrupt_handler_0x03
    dd interrupt_handler_0x04
    dd interrupt_handler_0x05
    dd interrupt_handler_0x06
    dd interrupt_handler_0x07
    dd interrupt_handler_0x08
    dd interrupt_handler_0x09
    dd interrupt_handler_0x0a
    dd interrupt_handler_0x0b
    dd interrupt_handler_0x0c
    dd interrupt_handler_0x0d
    dd interrupt_handler_0x0e
    dd interrupt_handler_0x0f
    dd interrupt_handler_0x10
    dd interrupt_handler_0x11
    dd interrupt_handler_0x12
    dd interrupt_handler_0x13
    dd interrupt_handler_0x14
    dd interrupt_handler_0x15
    dd interrupt_handler_0x16
    dd interrupt_handler_0x17
    dd interrupt_handler_0x18
    dd interrupt_handler_0x19
    dd interrupt_handler_0x1a
    dd interrupt_handler_0x1b
    dd interrupt_handler_0x1c
    dd interrupt_handler_0x1d
    dd interrupt_handler_0x1e
    dd interrupt_handler_0x1f
    dd interrupt_handler_0x20
    dd interrupt_handler_0x21
    dd interrupt_handler_0x22
    dd interrupt_handler_0x23
    dd interrupt_handler_0x24
    dd interrupt_handler_0x25
    dd interrupt_handler_0x26
    dd interrupt_handler_0x27
    dd interrupt_handler_0x28
    dd interrupt_handler_0x29
    dd interrupt_handler_0x2a
    dd interrupt_handler_0x2b
    dd interrupt_handler_0x2c
    dd interrupt_handler_0x2d
    dd interrupt_handler_0x2e
    dd interrupt_handler_0x2f

msg:
    db "interrupt_handler", 10, 0

exception_msg:
    db "exception_handler", 10, 0
