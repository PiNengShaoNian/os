#include "../include/asm/system.h"
#include "../include/linux/head.h"
#include "../include/linux/traps.h"

#define INTERRUPT_TABLE_SIZE    256

interrupt_gate_t interrupt_table[INTERRUPT_TABLE_SIZE] = {0};

xdt_ptr_t idt_ptr;

extern void interrupt_handler_entry();

extern void keymap_handler_entry();

extern void clock_handler_entry();

extern void hd_handler_entry();

extern void system_call_entry();

extern int interrupt_handler_table[0x2f];

void idt_init() {
    printk("init idt...\n");
    for (int i = 0; i < INTERRUPT_TABLE_SIZE; ++i) {
        interrupt_gate_t *p = &interrupt_table[i];

        int handler = (int) interrupt_handler_entry;

        if (i <= 0x15) {
            handler = (int) interrupt_handler_table[i];
        }

        if (i == 0x20) {
            handler = (int) clock_handler_entry;
        }
        if (i == 0x21) {
            handler = (int) keymap_handler_entry;
        }
        if (i == 0x2e) {
            handler = (int) hd_handler_entry;
        }
        if (i == 0x80) {
            handler = (int) system_call_entry;
        }

        p->offset0 = handler & 0xffff;
        p->offset1 = (handler >> 16) & 0xffff;
        p->selector = 1 << 3; // 代码段
        p->reserved = 0;      // 保留不用
        p->type = 0b1110;     // 中断门
        p->segment = 0;       // 系统段
        p->DPL = (i == 0x80) ? 3 : 0;           // 内核态
        p->present = 1;       // 有效
    }

    // 让CPU知道中断向量表
    write_xdt_ptr(&idt_ptr, INTERRUPT_TABLE_SIZE * 8, interrupt_table);

    asm volatile("lidt idt_ptr;");
}