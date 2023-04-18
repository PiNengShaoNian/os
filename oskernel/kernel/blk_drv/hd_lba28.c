#include "../../include/linux/hd.h"
#include "../../include/linux/fs.h"
#include "../../include/linux/kernel.h"
#include "../../include/linux/task.h"
#include "../../include/string.h"
#include "../../include/asm/io.h"
#include "../../include/assert.h"

extern task_t *wait_for_request;
extern hd_request_t g_hd_request;

dev_handler_fun_t dev_interrupt_handler;

// 根据此参数的值来决定硬盘中断的处理逻辑
bool dev_handler_fast = false;

/**
 * 获取发送硬盘操作请求后硬盘的状态
 * 准备就绪返回0，否则返回错误寄存器中存储的值
 */
static int win_result() {
    int i = in_byte(HD_STATUS);

//    printk("[%s:%d]hd status: 0x%x\n", __FUNCTION__, __LINE__, i);

    if ((i & (BUSY_STAT | READY_STAT | WRERR_STAT | SEEK_STAT | ERR_STAT))
        == (READY_STAT | SEEK_STAT))
        return 0; /* ok */

    if (i & 1) {
        i = in_byte(HD_ERROR);
    }

    return i;
}

hd_t *get_hd_info(u8 dev) {
    buffer_head_t *bh = kmalloc(sizeof(buffer_head_t));

    bh->data = kmalloc(512);
    bh->dev = dev;
    bh->sector_from = 0;
    bh->sector_count = 1;

    ll_rw_block(CHECK, bh);

    // 中断例程唤醒后才会执行
    hd_t *hd = kmalloc(sizeof(hd_t));

    // 从identify返回结果中取出硬盘信息
    memcpy(hd->number, bh->data + 10 * 20, 2 * 10);
    hd->number[20] = '\0';

    memcpy(hd->model, bh->data + 2 * 27, 2 * 20);
    hd->model[40] = '\0';

    hd->sectors = *(int *) (bh->data + 60 * 2);

    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));

    // 不要调用task_block阻塞任务。调用了也执行不到。所以获取硬盘信息的时候，在中断例程中阻塞任务及唤醒
    return hd;
}

void print_disk_info(hd_t *info) {
    printk("===== Hard Disk Info Start =====\n");
    printk("Hard disk Serial number: %s\n", info->dev_no);
    printk("Drive model: %s\n", info->model);
    printk("Hard disk size: %d sectors, %d M\n", info->sectors, info->sectors * 512 / 1024 / 1024);
    printk("===== Hard Disk Info End =====\n");
}

void read_intr() {
//    printk("[%s:%d]run...\n", __FUNCTION__, __LINE__);

    int status;
    if ((status = win_result()) != 0) {
        panic("identify disk error: %d\n", status);
    }

    port_read(HD_DATA, g_hd_request.buffer, 512 * g_hd_request.nr_sectors / 2);

    task_unblock(wait_for_request);
}

void write_intr() {
    if ((g_hd_request.bh->handler_state = win_result()) != 0) {
        panic("write disk error: %d\n", g_hd_request.bh->handler_state);
    }

    task_unblock(wait_for_request);
}

void do_identify() {
//    printk("[%s:%d]run...\n", __FUNCTION__, __LINE__);

    int status = 0;
    if ((status = win_result()) != 0) {
        panic("identify disk error: %d\n", status);
    }

    port_read(HD_DATA, g_hd_request.buffer, 512 * g_hd_request.nr_sectors / 2);

    task_unblock(wait_for_request);
}

static void hd_out(char hd, int from, int count, unsigned int cmd, dev_handler_fun_t handler) {
    // 这个得放在向硬盘发起请求的前面，否则中断例程中用的时候是没值的
    dev_interrupt_handler = handler;

    out_byte(HD_NSECTOR, count);
    out_byte(HD_SECTOR, from & 0xFF);
    out_byte(HD_LCYL, from >> 8 & 0xFF);
    out_byte(HD_HCYL, from >> 16 & 0xFF);
    out_byte(HD_CURRENT, 0b11100000 | (hd << 4) | (from >> 24 & 0xf));
    out_byte(HD_COMMAND, cmd);
}

void do_hd_request() {
    dev_handler_fast = false;

    switch (g_hd_request.cmd) {
        case READ:
            hd_out(g_hd_request.dev, g_hd_request.sector, g_hd_request.nr_sectors, WIN_READ, read_intr);
            break;
        case WRITE:
            hd_out(g_hd_request.dev, g_hd_request.sector, g_hd_request.nr_sectors, WIN_WRITE, write_intr);

            // 检测硬盘是否已准备好写操作
            int r = 0;
            for (int i = 0; i < 3000 && !(r = in_byte(HD_STATUS) & DRQ_STAT); i++);
            if (!r) {
                panic("Failed to write to the hard disk");
            }

            port_write(HD_DATA, g_hd_request.buffer, 256);
            break;
        case CHECK:
            dev_handler_fast = true;
            hd_out(g_hd_request.dev, g_hd_request.sector, g_hd_request.nr_sectors, 0xec, do_identify);
            break;
        default:
            break;
    }
}