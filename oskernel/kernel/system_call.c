#include "../include/linux/kernel.h"
#include "../include/asm/system.h"
#include "../include/linux/tty.h"
#include "../include/linux/sys.h"
#include "../include/linux/task.h"
#include "../include/linux/sched.h"
#include "../include/shell.h"

#define SYSTEM_CALL_TABLE_SIZE 64

extern task_t *current;

void *system_call_table[SYSTEM_CALL_TABLE_SIZE] = {
        sys_write,
        sys_exit,
        sys_fork,
        sys_get_pid,
        sys_get_ppid,
        sys_active_shell
};

ssize_t sys_write(int fd, const void *buf, size_t count) {
    return console_write((char *) buf, count);
}

int sys_exit(int status) {
    current_task_exit(status);
    sched();

    return 0;
}

pid_t sys_get_pid() {
    return get_task_pid(current);
}

pid_t sys_get_ppid() {
    return get_task_ppid(current);
}

void sys_active_shell() {
    active_shell();
}