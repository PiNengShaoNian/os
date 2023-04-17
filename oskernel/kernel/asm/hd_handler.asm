[bits 32]
[SECTION .text]

extern printk

global hd_handler_entry
hd_handler_entry:
    push msg
    call printk
    add esp, 4

    iret

msg:
    db "hd_handler_entry", 10, 0