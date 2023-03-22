#include "../include/asm/system.h"
#include "../include/linux/head.h"
#include "../include/linux/traps.h"

#define INTERRUPT_TABLE_SIZE    256

interrupt_gate_t interrupt_table[INTERRUPT_TABLE_SIZE] = {0};

char idt_ptr[6] = {0};

extern void interrupt_handler();

void idt_init() {
    for (int i = 0; i < INTERRUPT_TABLE_SIZE; ++i) {
        interrupt_gate_t *p = &interrupt_table[i];

        int handler = interrupt_handler;

        p->offset0 = handler & 0xffff;
        p->offset1 = (handler >> 16) & 0xffff;
        p->selector = 1 << 3; // 代码段
        p->reserved = 0;      // 保留不用
        p->type = 0b1110;     // 中断门
        p->segment = 0;       // 系统段
        p->DPL = 0;           // 内核态
        p->present = 1;       // 有效
    }

    // 让CPU知道中断向量表
    *(short *) idt_ptr = INTERRUPT_TABLE_SIZE * 8;
    *(int *) (idt_ptr + 2) = interrupt_table;

    BOCHS_DEBUG_MAGIC
    BOCHS_DEBUG_MAGIC

    asm volatile("lidt idt_ptr;");
}