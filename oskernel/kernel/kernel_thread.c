#include "../include/linux/kernel.h"
#include "../include/linux/hd.h"
#include "../include/linux/fs.h"
#include "../include/linux/task.h"
#include "../include/shell.h"

extern task_t *current;

task_t *wait_for_request = NULL;

void kernel_thread_fun(void *arg) {
    hd_init();

    char str[512] = "hello world!";
    bwrite(1, 0, str, 512);

    // 打印硬盘信息
    buffer_head_t *buff1 = bread(1, 0, 1);
    printk("%s\n", buff1->data);
    kfree_s(buff1->data, 512);
    kfree_s(buff1, sizeof(buffer_head_t));

    active_shell();

    while (true);
}