#include "../include/asm/system.h"
#include "../include/linux/kernel.h"
#include "../include/linux/traps.h"
#include "../include/string.h"

#define GDT_SIZE 256

u64 gdt[GDT_SIZE] = {0};

xdt_ptr_t gdt_ptr;

int r3_code_selector;
int r3_data_selector;

static void r3_gdt_code_item(int gdt_index, int base, int limit) {
    // 在实模式时已经构建了4个全局描述符，所以从4开始
    if (gdt_index < 4) {
        printk("the gdt_index:%d has been used...\n", gdt_index);
        return;
    }

    gdt_item_t* item = &gdt[gdt_index];

    item->limit_low = limit & 0xffff;
    item->base_low = base & 0xffffff;
    item->type = 0b1000;
    item->segment = 1;
    item->DPL = 0b11;
    item->present = 1;
    item->limit_high = limit >> 16 & 0xf;
    item->available = 0;
    item->long_mode = 0;
    item->big = 1;
    item->granularity = 1;
    item->base_high = base >> 24 & 0xff;
}

static void r3_gdt_data_item(int gdt_index, int base, int limit) {
    // 在实模式时已经构建了4个全局描述符，所以从4开始
    if (gdt_index < 4) {
        printk("the gdt_index:%d has been used...\n", gdt_index);
        return;
    }

    gdt_item_t* item = &gdt[gdt_index];

    item->limit_low = limit & 0xffff;
    item->base_low = base & 0xffffff;
    item->type = 0b0010;
    item->segment = 1;
    item->DPL = 0b11;
    item->present = 1;
    item->limit_high = limit >> 16 & 0xf;
    item->available = 0;
    item->long_mode = 0;
    item->big = 1;
    item->granularity = 1;
    item->base_high = base >> 24 & 0xff;
}

void gdt_init() {
    printk("init gdt...\n");

    // 取出当前的gdt表的base和limit到gdt_ptr中
    __asm__ volatile ("sgdt gdt_ptr;");

    // 复制先前在汇编中设置的前几项gdt表项，到新的gdt表中
    memcpy(&gdt, gdt_ptr.base, gdt_ptr.limit);

    // 创建r3模式下使用的段描述符: 代码段, 数据段
    r3_gdt_code_item(4, 0, 0xfffff);
    r3_gdt_data_item(5, 0, 0xfffff);



    // 创建r3模式下使用的段选择子: 代码段, 数据段
    /*
     * Index：在GDT数组或LDT数组的索引号(3~15位)
     * TI：Table Indicator，这个值为0表示查找GDT，1则查找LDT
     * RPL：请求特权级。以什么样的权限去访问段
     */
    // 0-1: RPL
    // 2: TI
    // 3-11: Index
    // 0b000000000000
    r3_code_selector = 4 << 3 | 0b011;
    r3_data_selector = 5 << 3 | 0b011;

    gdt_ptr.base = &gdt;
    // 这里要减1取[0,limit]而不是[0,limit)
    gdt_ptr.limit = sizeof(gdt) - 1;

    BOCHS_DEBUG_MAGIC

    __asm__ volatile ("lgdt gdt_ptr;");
}