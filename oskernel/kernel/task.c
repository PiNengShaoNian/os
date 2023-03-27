#include "../include/linux/kernel.h"
#include "../include/linux/task.h"
#include "../include/linux/mm.h"

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

task_t *create_task(char *name, task_fun_t fun) {
    task_union_t *task = (task_union_t *) get_free_page();
    memset(task, 0, PAGE_SIZE);

    task->task.pid = find_empty_process();
    tasks[task->task.pid] = &(task->task);

    task->task.cr3 = virtual_memory_init();

    return task;
}

void *idle(void *arg) {

}

void task_init() {
    create_task("idle", idle);
}
