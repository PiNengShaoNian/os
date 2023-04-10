[BITS 32]
[SECTION .text]

extern current

; 切idle任务专用
global switch_idle_task
switch_idle_task:
    ; 恢复上下文
    mov eax, [current]

    ; 恢复ebp0 esp0
    mov esp, [eax + 4]
    mov ebp, [eax + 15 * 4]

    ; 恢复通用寄存器
    mov ecx, [eax + 11 * 4]
    mov edx, [eax + 12 * 4]
    mov ebx, [eax + 13 * 4]
    mov esi, [eax + 16 * 4]
    mov edi, [eax + 17 * 4]

    mov eax, [eax + 8 * 4]      ; eip

    sti

    jmp eax

    ; 下面这两句正常情况执行不到,一种保险策略,以防万一
    sti
    hlt


