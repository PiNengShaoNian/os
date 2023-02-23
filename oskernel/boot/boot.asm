; 0柱面0磁道1扇区
[ORG  0x7c00]

[SECTION .data]
BOOT_MAIN_ADDR equ 0x500

[SECTION .text]
[BITS 16]
global _start
_start:
    ; 设置屏幕模式为文本模式，清除屏幕
    mov ax, 3
    int 0x10

    ; 将bootsect读入0x7e00
    ; 读盘
    mov ch, 0  ; 0 柱面
    mov dh, 0  ; 0 磁头
    mov cl, 2  ; 2 扇区
    mov bx, BOOT_MAIN_ADDR  ; 读出来的数据放在哪

    mov ah, 0x02  ; 读盘操作
    mov al, 1  ; 连续读几个扇区
    mov dl, 0  ; 驱动器编号

    int 0x13

    ; 跳到load好的setup
    mov si, jmp_to_setup
    call print

    xchg bx, bx

    jmp BOOT_MAIN_ADDR

read_floppy_error:
    mov si, read_floppy_error_msg
    call print

    jmp $

; 如何调用
; mov     si, msg   ; 1 传入字符串
; call    print     ; 2 调用
print:
    mov ah, 0x0e
    mov bh, 0
    mov bl, 0x01
.loop:
    mov al, [si]
    cmp al, 0
    jz .done
    int 0x10

    inc si
    jmp .loop
.done:
    ret

read_floppy_error_msg:
    db "read floppy error!", 10, 13, 0

jmp_to_setup:
    db "jump to setup...", 10, 13, 0

times 510 - ($ - $$) db 0
db 0x55, 0xaa