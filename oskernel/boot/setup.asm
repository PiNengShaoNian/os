; 0柱面0磁道2扇区
[ORG 0x500]

[SECTION .text]
[BITS 16]
global _start
_start:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov si, ax

    mov si, msg
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

msg:
    db "hello", 10, 13, 0