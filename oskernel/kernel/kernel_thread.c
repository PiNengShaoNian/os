#include "../include/linux/kernel.h"
#include "../include/linux/hd.h"
#include "../include/linux/task.h"
#include "../include/shell.h"

extern task_t *current;

task_t *wait_for_request = NULL;

void kernel_thread_fun(void *arg) {
    hd_init();

    wait_for_request = current;
    task_block(wait_for_request);

    active_shell();

    while (true);
}