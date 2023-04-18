#include "../include/linux/kernel.h"
#include "../include/linux/hd.h"
#include "../include/linux/fs.h"
#include "../include/linux/task.h"
#include "../include/shell.h"

extern task_t *current;
extern hd_t *g_active_hd;

task_t *wait_for_request = NULL;

void kernel_thread_fun(void *arg) {
    hd_init();

    // 获取hdb盘的信息
    init_active_hd_info(g_active_hd->dev_no);
    init_active_hd_partition();

    // 为每个分区创建超级块
    init_super_block();

    active_shell();

    while (true);
}