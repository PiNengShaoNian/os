[bits 32]
[SECTION .text]

global in_byte
in_byte:
    push ebp
    mov ebp, esp
                         ; 栈中的内容
    xor eax, eax         ; ebp + 8   --->  first arg
                         ; ebp + 4   --->  return address
    mov edx, [ebp + 8]   ; ebp       --->  prev ebp
    in al, dx            ; 将该函数第一个参数作为读端口，读取出来的数据放到al中

    leave
    ret

global out_byte
out_byte:
    push ebp
    mov ebp, esp

    mov edx, [ebp + 8]   ; port
    mov eax, [ebp + 12]  ; value
    out dx, al           ; 将一个byte送到dx中的端口中

    leave
    ret

global in_word
in_word:
    push ebp
    mov ebp, esp

    xor eax, eax

    mov edx, [ebp + 8]   ; port
    in ax, dx

    leave
    ret

global out_word
out_word:
    push ebp
    mov ebp, esp

    mov edx, [ebp + 8]    ; port
    mov eax, [ebp + 12]   ; value
    out dx, ax

    leave
    ret