#include "../include/linux/kernel.h"
#include "../include/asm/system.h"
#include "../include/linux/tty.h"
#include "../include/linux/sys.h"
#include "../include/linux/task.h"
#include "../include/linux/sched.h"

#define SYSTEM_CALL_TABLE_SIZE 64

extern task_t *current;

void *system_call_table[SYSTEM_CALL_TABLE_SIZE] = {
        sys_write
};

ssize_t sys_write(int fd, const void *buf, size_t count) {
    return console_write((char *) buf, count);
}