[bits 32]
[SECTION .text]

extern printk

global interrupt_handler
interrupt_handler:
    push msg
    call printk
    add esp, 4

    iret

msg:
    db "interrupt_handler", 10, 0