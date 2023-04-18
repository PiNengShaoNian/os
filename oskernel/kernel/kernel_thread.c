#include "../include/linux/kernel.h"
#include "../include/linux/hd.h"
#include "../include/linux/fs.h"
#include "../include/linux/task.h"
#include "../include/shell.h"

extern task_t *current;

task_t *wait_for_request = NULL;

void kernel_thread_fun(void *arg) {
    hd_init();

    buffer_head_t *bh = bread(0, 0, 1);
    printk("read disk: %x\n", (uchar) bh->data[0]);

    active_shell();

    while (true);
}