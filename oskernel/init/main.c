#include "../include/linux/tty.h"
#include "../include/linux/kernel.h"
#include "../include/linux/traps.h"
#include "../include/linux/mm.h"
#include "../include/linux/task.h"
#include "../include/stdio.h"
#include "../include/string.h"

extern void clock_init();

void user_mode() {
    int age = 10;

    char *str = "hello world!";
    printf("%s, %d\n", str, 11);

    while(true);
}

void kernel_main(void) {
    console_init();
    gdt_init();
    idt_init();
    clock_init();

    memory_init();
    memory_map_init();

    print_check_memory_info();

    task_init();

    __asm__("sti;");

    while (true);
}
