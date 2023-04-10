#include "../include/linux/kernel.h"
#include "../include/linux/task.h"
#include "../include/linux/mm.h"

extern task_t *current;

task_t *tasks[NR_TASKS] = {0};

static int find_empty_process() {
    int ret = 0;

    bool is_finded = false;

    for (int i = 0; i < NR_TASKS; ++i) {
        if (tasks[i] == 0) {
            is_finded = true;

            ret = i;
            break;
        }
    }

    if (!is_finded) {
        printk("no valid pid\n");
        return -1;
    }

    return ret;
}

task_t *create_task(char *name, task_fun_t fun, int priority) {
    task_union_t *task = (task_union_t *) kmalloc(4096);
    memset(task, 0, PAGE_SIZE);

    task->task.pid = find_empty_process();
    task->task.ppid = current == NULL ? 0 : current->pid;

    task->task.scheduling_times = 0;
    strcpy(task->task.name, name);

    tasks[task->task.pid] = &(task->task);

    task->task.tss.cr3 = (int) task + sizeof(task_t);
    task->task.tss.eip = fun;

    // r0 stack
    task->task.esp0 = (int) task + PAGE_SIZE;
    task->task.ebp0 = task->task.esp0;

    task->task.tss.esp0 = task->task.esp0;

    task->task.state = TASK_READY;

    return task;
}

void *idle(void *arg) {
    printk("#1 idle task running...\n");

    while (true) {
        printk("#2 idle task running...\n");

        __asm__ volatile ("sti;");
        __asm__ volatile ("hlt;");
    }
}

void task_init() {
    create_task("idle", idle, 1);
}

int inc_scheduling_times(task_t *task) {
    return task->scheduling_times++;
}

pid_t get_task_pid(task_t *task) {
    return task->pid;
}

pid_t get_task_ppid(task_t *task) {
    return task->ppid;
}
