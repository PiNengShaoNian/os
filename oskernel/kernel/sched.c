#include "../include/linux/sched.h"
#include "../include/linux/task.h"
#include "../include/linux/kernel.h"
#include "../include/asm/system.h"

extern void switch_task(task_t *task);

extern void switch_idle_task(task_t *task);

extern task_t *tasks[NR_TASKS];

task_t *current = NULL;

task_t *find_ready_task() {
    task_t *next = NULL;

    for (int i = 1; i < NR_TASKS; ++i) {
        task_t *task = tasks[i];

        if (task == NULL) continue;
        if (task->state != TASK_READY) continue;

        next = task;
    }

    return next;
}

void sched() {
    task_t *next = find_ready_task();

    if (next == NULL) {
        current = tasks[0];

        switch_idle_task(tasks[0]);

        return;
    }

    next->state = TASK_RUNNING;

    current = next;

    switch_task(next);
}

void do_timer() {
    sched();
}