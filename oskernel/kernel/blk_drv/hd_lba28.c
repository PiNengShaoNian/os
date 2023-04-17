#include "../../include/linux/hd.h"
#include "../../include/linux/kernel.h"
#include "../../include/asm/io.h"

dev_handler_fun_t dev_interrupt_handler;

void do_identify() {
    printk("%s\n", __FUNCTION__);
}

void hd_out() {
    char hd = 0;
    int from = 0;
    int count = 1;
    unsigned int cmd = 0xec;

    // 这个得放在向硬盘发起请求的前面，否则中断例程中用的时候是没值的
    dev_interrupt_handler = do_identify;

    out_byte(HD_NSECTOR, count);
    out_byte(HD_SECTOR, from & 0xFF);
    out_byte(HD_LCYL, from >> 8 & 0xFF);
    out_byte(HD_HCYL, from >> 16 & 0xFF);
    out_byte(HD_CURRENT, 0b11100000 | (hd << 4) | (from >> 24 & 0xf));
    out_byte(HD_COMMAND, cmd);
}