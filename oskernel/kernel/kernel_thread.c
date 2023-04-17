#include "../include/linux/kernel.h"
#include "../include/linux/hd.h"
#include "../include/shell.h"

void kernel_thread_fun(void *arg) {
    hd_init();

    active_shell();

    while (true);
}