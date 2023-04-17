#include "../include/linux/kernel.h"
#include "../include/shell.h"

void kernel_thread_fun(void *arg) {
    active_shell();

    while (true);
}