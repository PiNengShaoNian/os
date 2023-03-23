#include "../include/asm/io.h"
#include "../include/linux/traps.h"

#define PIC_M_CTRL  0x20    // 主片的控制端口
#define PIC_M_DATA  0x21    // 主片的数据端口
#define PIC_S_CTRL  0xa0    // 从片的控制端口
#define PIC_S_DATA  0xa1    // 从片的数据端口
#define PIC_EOI     0x20    // 通知中断控制器中断结束

void send_eoi(int idt_index) {
    if (idt_index >= 0x20 && idt_index < 0x28) {
        out_byte(PIC_M_CTRL, PIC_EOI);
    } else if (idt_index >= 0x28 && idt_index < 0x30) {
        out_byte(PIC_M_CTRL, PIC_EOI);
        out_byte(PIC_S_CTRL, PIC_EOI);
    }
}

void write_xdt_ptr(xdt_ptr_t* p, short limit, int base) {
    p->limit = limit;
    p->base = base;
}
