[bits 32]
[SECTION .text]

extern printk
extern dev_interrupt_handler
extern dev_handler_fast
extern current
extern wait_for_request
extern set_block

global hd_handler_entry
hd_handler_entry:
    ; 读写硬盘会跳走,0xec会按顺序执行
    push ecx            ; 后面要用到ecx，把当前的ecx保存起来，以防用的时候覆盖了出现问题
    mov ecx, [dev_handler_fast]
    cmp ecx, 1
    pop ecx
    jne .call_handler   ; 不等于0表示慢速响应（读写硬盘的时候才会出现）

    push ecx
    mov ecx, [current]  ; 如果current为空就报错
    cmp ecx, 0
    je .hd_handler_fail

    ; 给wait_for_request赋值
    mov [wait_for_request], ecx

.save_env:
    mov [ecx + 10 * 4], eax
    mov [ecx + 12 * 4], edx
    mov [ecx + 13 * 4], ebx
    mov [ecx + 15 * 4], ebp
    mov [ecx + 16 * 4], esi
    mov [ecx + 17 * 4], edi

    mov eax, [esp + 4]          ; eip
    mov [ecx + 8 * 4], eax      ; tss.eip

    mov eax, esp
    add eax, 0x10
    mov [ecx + 4], eax          ; tss.esp0

    mov eax, ecx
    pop ecx
    mov [eax + 11 * 4], ecx

    ; 设置任务状态为block
.block_task:
    push eax
    call set_block
    add esp, 4

.call_handler:
    call [dev_interrupt_handler]

    iret

.hd_handler_fail:
    pop ecx

    push fail_msg
    call printk
    add esp, 4

    iret

fail_msg:
    db "hd handler fail", 10, 0

msg:
    db "hd_handler_entry", 10, 0
