#ifndef OSKERNEL_TASK_H
#define OSKERNEL_TASK_H

#include "types.h"
#include "mm.h"
#include "fs.h"

// 进程上限
#define NR_TASKS 64

#define NR_OPEN 16

typedef void *(*task_fun_t)(void *);

typedef enum task_state_t {
    TASK_INIT,     // 初始化
    TASK_RUNNING,  // 执行
    TASK_READY,    // 就绪
    TASK_BLOCKED,  // 阻塞
    TASK_SLEEPING, // 睡眠
    TASK_WAITING,  // 等待
    TASK_DIED,     // 死亡
} task_state_t;

typedef struct tss_t {
    u32 backlink; // 前一个任务的链接，保存了前一个任状态段的段选择子
    u32 esp0;     // ring0 的栈顶地址
    u32 ss0;      // ring0 的栈段选择子
    u32 esp1;     // ring1 的栈顶地址
    u32 ss1;      // ring1 的栈段选择子
    u32 esp2;     // ring2 的栈顶地址
    u32 ss2;      // ring2 的栈段选择子
    u32 cr3;
    u32 eip;
    u32 flags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u32 es;
    u32 cs;
    u32 ss;
    u32 ds;
    u32 fs;
    u32 gs;
    u32 ldtr;          // 局部描述符选择子
    u16 trace: 1;     // 如果置位，任务切换时将引发一个调试异常
    u16 reversed: 15; // 保留不用
    u16 iobase;        // I/O 位图基地址，16 位从 TSS 到 IO 权限位图的偏移
    u32 ssp;           // 任务影子栈指针
} __attribute__((packed)) tss_t;

typedef struct task_t {
    tss_t tss;
    int pid;
    int ppid;
    char name[32];
    task_state_t state;
    int exit_code;
    int counter;
    int priority;
    int scheduling_times;       // 调度次数
    int esp0;                   // 刚开始创建的时候 活动的esp3保存在tss中
    int ebp0;
    int esp3;
    int ebp3;
    // 读硬盘的时候，有可能先阻塞任务再进入硬盘中断，这是正常情况，这个属性用不上
    // 还有一种情况就是还未阻塞任务就进入了硬盘中断，所以需要这个属性来判断
    bool resume_from_irq;

    file_t *file_descriptor[NR_OPEN];

    dir_entry_t *current_active_dir;
    m_inode_t *current_active_dir_inode;

    dir_entry_t *root_dir;
    m_inode_t *root_dir_inode;

    int magic;
} task_t;

typedef union task_union_t {
    task_t task;
    char stack[PAGE_SIZE];
} task_union_t;

task_t *create_task(char *name, task_fun_t fun, int priority);

void task_init();

// 参数位置不可变，head.asm中有调用
void task_exit(int code, task_t *task);

void current_task_exit(int code);

void task_sleep(int ms);

void task_wakeup();

int inc_scheduling_times(task_t *task);

pid_t get_task_pid(task_t *task);

pid_t get_task_ppid(task_t *task);

task_t *create_child(char *name, task_fun_t fun, int priority);

int get_esp3(task_t *task);

void set_esp3(task_t *task, int esp);

void task_block(task_t *task);

void task_unblock(task_t *task);

void set_block(task_t *task);

bool is_blocked(task_t *task);

int find_empty_file_descriptor();

#endif // OSKERNEL_TASK_H