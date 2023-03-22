[SECTION .text]
[BITS 32]
extern kernel_main

global _start
_start:
; 配置8259a芯片，响应中断用
.config_8a59a:
    ; 向主发送ICW1
    mov al, 11h
    out 20h, al

    ; 向从发送ICW1
    out 0a0h, al

    ; 向主发送ICW2
    mov al, 20h
    out 21h, al

    ; 向从发送ICW2
    mov al, 28h
    out 0a1h, al

    ; 向主发送ICW3
    mov al, 04h
    out 21h, al

    ; 向从发送ICW3
    mov al, 02h
    out 0A1h , al

    ; 向主发送ICW4
    mov al, 003h
    out 021h, al

    ; 向从发送ICW4
    out 0A1h, al

    ; 屏蔽所有中断，只接收键盘中断
.enable_8259a_main:
    mov al, 11111101b
    out 21h, al

    ; 屏蔽从芯片所有中断响应
.disable_8259a_slave:
    mov al, 11111111b
    out 0A1h, al

    ; 调用c程序
.enter_c_word:
    call kernel_main

    jmp $