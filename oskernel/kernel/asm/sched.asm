[BITS 32]
[SECTION .text]

extern task_exit
extern sched
extern inc_scheduling_times
extern get_task_ppid

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

; return address
; 参数
global switch_task
switch_task:
    ; 恢复上下文
    mov eax, [current]

    ; 恢复ebp0 esp0
    mov esp, [eax + 4]
    mov ebp, [eax + 15 * 4]

    push eax                        ; 获取父进程ID，如果不为0表示子进程，不需要压入task_exit_handler
    call get_task_ppid
    add esp, 4
    cmp eax, 0
    jne .recover_env

    mov eax, [current]
    push eax
    call inc_scheduling_times
    add esp, 4

    cmp eax, 0
    jne .recover_env                ; 不是第一次调度

    ; 如果是第一次調度
    mov eax, task_exit_handler
    push eax

.recover_env:
    mov eax, [current]

    ; 恢复通用寄存器
    mov ecx, [eax + 11 * 4]
    mov edx, [eax + 12 * 4]
    mov ebx, [eax + 13 * 4]
    mov esi, [eax + 16 * 4]
    mov edi, [eax + 17 * 4]

    mov eax, [eax + 8 * 4]      ; eip

    sti

    jmp eax

task_exit_handler:
    mov eax, [current]
    push eax
    push 0
    call task_exit
    add esp, 8

    call sched

    ; 下面这两句正常情况执行不到,一种保险策略
    sti
    hlt



