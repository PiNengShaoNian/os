[bits 32]
[SECTION .text]

extern printk
extern dev_interrupt_handler

global hd_handler_entry
hd_handler_entry:
    call [dev_interrupt_handler]

    iret

msg:
    db "hd_handler_entry", 10, 0