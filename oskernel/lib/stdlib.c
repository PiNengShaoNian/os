#include "../include/unistd.h"
#include "../include/stdlib.h"

void _exit(int exit_code) {
    __asm__ __volatile__ ("int 0x80"::"a" (__NR_exit), "b" (exit_code));
}