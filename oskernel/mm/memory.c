#include "../include/asm/system.h"
#include "../include/linux/mm.h"
#include "../include/linux/kernel.h"

#define ARDS_ADDR 0x1100
#define LOW_MEM 0x100000 // 1M以下的物理内存给内核用

#define ZONE_VALID 1 // ards 可用内存区域
#define ZONE_RESERVED // ards 不可用区域

void print_check_memory_info() {
    check_memory_info_t *p = (check_memory_info_t *)ARDS_ADDR;
    check_memory_item_t *p_data = (check_memory_item_t *)(ARDS_ADDR + 2);

    unsigned short times = p->times;

    for(int i = 0; i < times; ++i) {
        check_memory_item_t *tmp = p_data + i;

        printk("%x, %x, %x, %x, %d\n", tmp->base_addr_high, tmp->base_addr_low,
               tmp->length_high, tmp->length_low, tmp->type);
    }
}