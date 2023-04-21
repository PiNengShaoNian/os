#include "../include/unistd.h"

pid_t getpid() {
    pid_t pid = 0;

    __asm__ __volatile__ ("int 0x80":"=a"(pid):"0"(__NR_get_pid));

    return pid;
}

pid_t getppid() {
    pid_t pid = 0;

    __asm__ __volatile__ ("int 0x80":"=a"(pid):"0"(__NR_get_ppid));

    return pid;
}

void uactive_shell() {
    __asm__ __volatile__ ("int 0x80"::"a"(__NR_active_shell));
}

FILE *fopen(const char *filename, const char *mode) {
    FILE *file = NULL;

    __asm__ __volatile__ ("int 0x80":"=a"(file):"0"(__NR_fopen), "b"(filename), "c"(mode));

    return file;
}

int fclose(FILE *stream) {
    int ret;

    __asm__ __volatile__ ("int 0x80":"=a"(ret): "0"(__NR_fclose), "b"(stream));

    return ret;
}