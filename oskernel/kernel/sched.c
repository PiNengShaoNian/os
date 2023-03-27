#include "../include/linux/sched.h"
#include "../include/linux/task.h"
#include "../include/linux/kernel.h"
#include "../include/asm/system.h"

extern task_t *tasks[NR_TASKS];

task_t *current = NULL;

void sched() {
    current = tasks[0];
}