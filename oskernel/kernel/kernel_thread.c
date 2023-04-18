#include "../include/linux/kernel.h"
#include "../include/linux/hd.h"
#include "../include/linux/fs.h"
#include "../include/linux/task.h"
#include "../include/shell.h"

extern task_t *current;

task_t *wait_for_request = NULL;

void kernel_thread_fun(void *arg) {
    hd_init();

    hd_t *hd = get_hd_info(0);

    // 打印硬盘信息
    print_disk_info(hd);
    kfree_s(hd, sizeof(hd_t));

    active_shell();

    while (true);
}