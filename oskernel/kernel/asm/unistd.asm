[BITS 32]
[SECTION .text]

global fork
fork:
    mov eax, 2
    int 0x80

    ret