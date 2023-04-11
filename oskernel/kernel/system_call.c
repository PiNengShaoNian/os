#include "../include/linux/kernel.h"
#include "../include/asm/system.h"

void system_call() {
    printk("system_call\n");
}