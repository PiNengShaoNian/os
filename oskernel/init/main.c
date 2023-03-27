#include "../include/linux/tty.h"
#include "../include/linux/kernel.h"
#include "../include/linux/traps.h"
#include "../include/linux/mm.h"
#include "../include/linux/task.h"
#include "../include/linux/sched.h"

extern void clock_init();

void kernel_main(void) {
    console_init();
    gdt_init();
    idt_init();
    clock_init();

    memory_init();
    memory_map_init();

    print_check_memory_info();

    task_init();

    sched();

    // 测试分配虚拟内存
    kmalloc(1);

    __asm__("sti;");

    while (true);
}
