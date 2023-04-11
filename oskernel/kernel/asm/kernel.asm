[BITS 32]

extern user_mode
extern _exit
extern get_esp3

extern current

global move_to_user_mode

[SECTION .data]
R3_CODE_SELECTOR equ (4 << 3 | 0b11)
R3_DATA_SELECTOR equ (5 << 3 | 0b11)

[SECTION .text]

; eip
; cs
; eflags
; esp3
; ss3
move_to_user_mode:
    mov esi, [current]

    push esi
    call get_esp3
    add esp, 4

    push R3_DATA_SELECTOR       ; ss
    push eax                    ; esp
    pushf                       ; eflags

    mov ax, R3_CODE_SELECTOR
    push eax                    ; cs

    push user_mode_handler      ; eip

    mov ax, R3_DATA_SELECTOR
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    iretd

user_mode_handler:
    call user_mode

    push 0
    call _exit
    add esp, 4

    ; 下面这两句正常情况执行不到,一种保险策略
    sti
    hlt
