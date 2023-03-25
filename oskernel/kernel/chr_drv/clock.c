#include "../../include/linux/kernel.h"
#include "../../include/linux/traps.h"

void clock_handler(int idt_index) {
    send_eoi(idt_index);

    printk("0x%x\n", idt_index);
}