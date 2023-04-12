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
