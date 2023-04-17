#include "../../include/linux/hd.h"
#include "../../include/linux/kernel.h"
#include "../../include/linux/task.h"
#include "../../include/string.h"
#include "../../include/asm/io.h"
#include "../../include/assert.h"

extern task_t *wait_for_request;

dev_handler_fun_t dev_interrupt_handler;

// 根据此参数的值来决定硬盘中断的处理逻辑
bool dev_handler_fast = false;

/**
 * 获取发送硬盘操作请求后硬盘的状态
 * 准备就绪返回0，否则返回错误寄存器中存储的值
 */
static int win_result() {
    int i = in_byte(HD_STATUS);

    printk("[%s:%d]hd status: 0x%x\n", __FUNCTION__, __LINE__, i);

    if ((i & (BUSY_STAT | READY_STAT | WRERR_STAT | SEEK_STAT | ERR_STAT))
        == (READY_STAT | SEEK_STAT))
        return 0; /* ok */

    if (i & 1) {
        i = in_byte(HD_ERROR);
    }

    return i;
}

static void print_disk_info(hd_t info) {
    printk("===== Hard Disk Info Start =====\n");
    printk("Hard disk Serial number: %s\n", info.dev_no);
    printk("Drive model: %s\n", info.model);
    printk("Hard disk size: %d sectors, %d M\n", info.sectors, info.sectors * 512 / 1024 / 1024);
    printk("===== Hard Disk Info End =====\n");
}

void read_intr() {
    printk("[%s:%d]run...\n", __FUNCTION__, __LINE__);

    task_unblock(wait_for_request);
}

void do_identify() {
    printk("[%s:%d]run...\n", __FUNCTION__, __LINE__);

    int status = 0;
    if (status = win_result()) {
        panic("[%s:%d]read disk error: %d\n", __FUNCTION__, __LINE__, status);
    }

    char buf[512] = {0};

    // 读硬盘
    port_read(HD_DATA, buf, 512 / 2);

    hd_t hd;

    // 从identify返回结果中取出硬盘信息
    memcpy(&hd.number, buf + 10 * 20, 2 * 10);
    hd.number[20] = '\0';

    memcpy(&hd.model, buf + 2 * 27, 2 * 20);
    hd.model[40] = '\0';

    hd.sectors = *(int *) (buf + 60 * 2);

    // 打印硬盘信息
    print_disk_info(hd);
}

void hd_out() {
    char hd = 0;
    int from = 0;
    int count = 1;
    unsigned int cmd = WIN_READ;

    // 这个得放在向硬盘发起请求的前面，否则中断例程中用的时候是没值的
    dev_interrupt_handler = read_intr;

    out_byte(HD_NSECTOR, count);
    out_byte(HD_SECTOR, from & 0xFF);
    out_byte(HD_LCYL, from >> 8 & 0xFF);
    out_byte(HD_HCYL, from >> 16 & 0xFF);
    out_byte(HD_CURRENT, 0b11100000 | (hd << 4) | (from >> 24 & 0xf));
    out_byte(HD_COMMAND, cmd);
}