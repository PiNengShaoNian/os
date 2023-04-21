#include "../include/linux/tty.h"
#include "../include/linux/traps.h"
#include "../include/linux/mm.h"
#include "../include/linux/task.h"

extern void clock_init();

void kernel_main(void) {
    console_init();
    clock_init();

    print_check_memory_info();
    memory_init();
    memory_map_init();

    virtual_memory_init();

    gdt_init();
    idt_init();

    task_init();

    __asm__("sti;");

    while (true);
}
