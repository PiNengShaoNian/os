[BITS 32]
[SECTION .text]

extern create_child
extern get_task_pid
extern get_task_ppid

global sys_fork
sys_fork:
    cli                     ; 关中断

    push 10                 ; 创建子进程
    push .child_return
    push task_name
    call create_child
    add esp, 12

    ; ss3                                  esp + 4 * 8
    ; esp3                                 esp + 4 * 7
    ; eflags                               esp + 4 * 6
    ; cs                                   esp + 4 * 5
    ; eip                                  esp + 4 * 4
    ; edx                                  esp + 4 * 3
    ; ecx                                  esp + 4 * 2
    ; ebx                                  esp + 4 * 1
    ; return address(system_call_entry)    esp + 4 * 0

    ; 构建子进程的栈, 让他在内核态执行完.child_return后,在system_call_entry中返回，并接着通过调用iret进入用户态中
    ; 其中esp3要换成子进程自己的
.parent_run:
    mov edx, esp            ; 临时保存父进程的esp

    ; 将父进程栈中的ss、esp、eflags压入子进程的栈
    mov ebx, [esp + 4 * 8]  ; ss
    mov esi, [eax + 4 * 14] ; esp 子线程自己的esp
    mov edi, [esp + 4 * 6]  ; eflags

    mov esp, [eax + 4]      ; 切到子进程的栈
    push ebx
    push esi
    push edi

    mov [eax + 4], esp      ; 更新子进程的esp

    ; 将父进程栈中的cs、eip压入子进程的栈
    mov esp, edx
    mov ecx, [esp]          ; 获取父进程栈的第一个元素，即返回地址
    mov esi, [esp + 4 * 5]  ; cs
    mov edi, [esp + 4 * 4]  ; eip

    mov esp, [eax + 4]      ; 切到子进程的栈
    push esi
    push edi

    push 0                  ; 兼容system_call_entry中的add esp, 12
    push 0
    push 0

    push ecx                ; 将返回地址压入栈, 让自己从返回到system_call_entry中

    mov [eax + 4], esp      ; 更新子进程的esp

    ; 栈切回到父进程的
    mov esp, edx

    push eax                ; 获取子进程的ID并返回
    call get_task_pid
    add esp, 4

    sti

    ret

.child_return:
    mov eax, 0

    sti

    ret

task_name db "t1", 0
