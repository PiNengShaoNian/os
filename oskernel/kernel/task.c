#include "../include/linux/kernel.h"
#include "../include/linux/task.h"
#include "../include/linux/mm.h"
#include "../include/string.h"
#include "../include/assert.h"

extern void sched_task();

extern void kernel_thread_fun(void *arg);

extern task_t *current;
extern int jiffy;
extern int cpu_tickes;

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
    task->task.ppid = (current == NULL) ? 0 : current->pid;

    task->task.priority = priority;
    task->task.counter = priority;
    task->task.scheduling_times = 0;

    strcpy(task->task.name, name);

    // 加入tasks
    tasks[task->task.pid] = &(task->task);

    task->task.tss.cr3 = virtual_memory_init();
    task->task.tss.eip = (u32) fun;

    // r0 stack
    task->task.esp0 = (int) task + PAGE_SIZE;
    task->task.ebp0 = task->task.esp0;

    task->task.tss.esp0 = task->task.esp0;

    // r3 stack
    task->task.esp3 = kmalloc(4096) + PAGE_SIZE;
    task->task.ebp3 = task->task.esp3;

    task->task.tss.esp = task->task.esp3;
    task->task.tss.ebp = task->task.ebp3;

    task->task.state = TASK_READY;

    return task;
}

task_t *create_child(char *name, task_fun_t fun, int priority) {
    if (current == NULL) {
        printk("[%s:%d] current task is null\n", __FILE__, __LINE__);
        return NULL;
    }

    task_union_t *task = (task_union_t *) kmalloc(PAGE_SIZE);
    memset(task, 0, PAGE_SIZE);

    // 拷贝父进程的内存空间
    memcpy(task, current, PAGE_SIZE);

    // 设置个性化数据
    task->task.pid = find_empty_process();

    task->task.ppid = (current == NULL) ? 0 : current->pid;

    task->task.priority = priority;
    task->task.counter = priority;
    task->task.scheduling_times = 0;

    strcpy(task->task.name, name);

    // 加入tasks
    tasks[task->task.pid] = &(task->task);

    task->task.tss.cr3 = (int) task + sizeof(task_t);
    task->task.tss.eip = (u32) fun;

    // r0 stack
    task->task.esp0 = (int) task + PAGE_SIZE;
    task->task.ebp0 = task->task.esp0;

    task->task.tss.esp0 = task->task.esp0;

    // r3 stack
    task->task.esp3 = kmalloc(4096) + PAGE_SIZE;
    task->task.ebp3 = task->task.esp3;

    task->task.tss.esp = task->task.esp3;
    task->task.tss.ebp = task->task.ebp3;

    // 将父进程r3 stack拷贝到子进程r3 stack
    memcpy(task->task.esp3 - PAGE_SIZE, current->esp3 - PAGE_SIZE, PAGE_SIZE);

    // 计算子进程的esp
    int parent_esp_used = current->esp3 - current->tss.esp;
    int parent_ebp_used = current->ebp3 - current->tss.ebp;

    task->task.tss.esp -= parent_esp_used;
    task->task.tss.ebp -= parent_ebp_used;

    task->task.state = TASK_READY;

    return task;
}

void *idle(void *arg) {
    create_task("init", kernel_thread_fun, 1);

    while (true) {
//        printk("idle task running...\n");

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

void task_exit(int code, task_t *task) {
    for (int i = 1; i < NR_TASKS; ++i) {
        task_t *tmp = tasks[i];

        if (task == tmp) {
            printk("task exit: %s\n", tmp->name);

            tmp->exit_code = code;

            // 先移除，后面有父子进程再相应处理
            tasks[i] = NULL;

            current = NULL;

            kfree_s(tmp->esp3 - PAGE_SIZE, PAGE_SIZE);
            kfree_s(tmp, PAGE_SIZE);

            break;
        }
    }
}

void current_task_exit(int code) {
    for (int i = 1; i < NR_TASKS; ++i) {
        task_t *tmp = tasks[i];

        if (current == tmp) {
            printk("task exit: %s\n", tmp->name);

            tmp->exit_code = code;

            // 先移除，后面有父子进程再相应处理
            tasks[i] = NULL;

            current = NULL;

            kfree_s(tmp->esp3 - PAGE_SIZE, PAGE_SIZE);
            kfree_s(tmp, PAGE_SIZE);

            break;
        }
    }
}

void task_sleep(int ms) {
    CLI

    if (current == NULL) {
        printk("task sleep: current task is null\n");
        return;
    }

    int ticks = ms / jiffy;
    ticks += (ms % jiffy == 0) ? 0 : 1;

    current->counter = cpu_tickes + ticks;
    current->state = TASK_SLEEPING;

    sched_task();
}

void task_wakeup() {
    for (int i = 1; i < NR_TASKS; ++i) {
        task_t *task = tasks[i];

        if (task == NULL) continue;
        if (task->state != TASK_SLEEPING) continue;

        if (cpu_tickes >= task->counter) {
            task->state = TASK_READY;
            task->counter = task->priority;
        }
    }
}

int get_esp3(task_t *task) {
    return task->esp3;
}

void set_esp3(task_t *task, int esp) {
    task->tss.esp = esp;
}

void task_block(task_t *task) {
    if (task->resume_from_irq == true) {
        task->resume_from_irq = false;
        return;
    }

    task->state = TASK_BLOCKED;

    sched_task();
}

void task_unblock(task_t *task) {
    if (task->state != TASK_BLOCKED) {
        printk("[%s]task state: %d\n", __FUNCTION__, task->state);

        task->resume_from_irq = true;
        return;
    }

    task->state = TASK_READY;

    sched_task();
}

void set_block(task_t *task) {
    task->state = TASK_BLOCKED;
}

bool is_blocked(task_t *task) {
    return task->state == TASK_BLOCKED;
}

int find_empty_file_descriptor() {
    for (int i = 0; i < NR_OPEN; ++i) {
        file_t *file = &current->file_descriptor[i];
        if (file->f_mode == NULL) {
            return i;
        }
    }

    return -1;
}