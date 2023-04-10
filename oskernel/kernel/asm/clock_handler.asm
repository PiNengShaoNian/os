[bits 32]
[SECTION .txt]

extern clock_handler

extern current

; ecx
; eip
; cs
; eflags
global clock_handler_entry:
clock_handler_entry:
    push ecx

    mov ecx, [current]
    cmp ecx, 0
    je .call_handler

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

.call_handler:
    push 0x20
    call clock_handler
    add esp, 4

    iret

msg:
    db "clock_handler", 10, 0
