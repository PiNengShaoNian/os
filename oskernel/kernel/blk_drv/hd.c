#include "../../include/linux/hd.h"
#include "../../include/linux/fs.h"
#include "../../include/linux/mm.h"
#include "../../include/linux/kernel.h"
#include "../../include/string.h"
#include "../../include/asm/io.h"

// BIOS例程会将检测到的硬盘数量写在内存0x475位置（注意：0x475为物理地址，如果开启分页映射方式不对可能读不到）
#define HD_NUMBER_MEMORY_PTR    0x475

// IDE通道数量
#define IDE_CHANNEL_NUMBER  2

// 通常来说，主板有两个IDE磁盘通道，每个通道可以挂两个盘：主盘、从盘。qemu模拟的硬件环境也是如此
hd_channel_t g_hd_channel[IDE_CHANNEL_NUMBER];

// 共有多少块硬盘（以全局变量保存下来，后面要用
uint g_hd_number = 0;

// 活跃的硬盘。后面的文件系统就针对这个盘做操作，先只设置一个盘
hd_t *g_active_hd = NULL;

static void _ide_channel_init() {
    for (int i = 0; i < IDE_CHANNEL_NUMBER; ++i) {
        hd_channel_t *channel = &g_hd_channel[i];

        if (i == 0) {
            channel->port_base = 0x1f0;
            channel->irq_no = 0x20 + 14;
        } else {
            channel->port_base = 0x170;
            channel->irq_no = 0x20 + 15;
        }
    }
}

static void _hd_init() {
    /**
     * 取到当前挂的硬盘数量
     * 第1、2块盘对应的是通道1的主从盘，第3、4块盘对应的是通道2的主从盘。第5块盘qemu就不支持了
     */
    u8 hd_number = *(u8 *) HD_NUMBER_MEMORY_PTR;

    g_hd_number = hd_number;

    printk("disk number: %d\n", hd_number);

    for (int i = 0; i < hd_number; ++i) {
        hd_t *hd;

        switch (i) {
            case 0:
                hd = &g_hd_channel[0].hd[0];

                hd->dev_no = i;
                hd->is_master = 1;
                hd->channel = &g_hd_channel[0];

                break;
            case 1:
                hd = &g_hd_channel[0].hd[1];

                hd->dev_no = i;
                hd->is_master = 0;
                hd->channel = &g_hd_channel[0];

                break;
            case 2:
                hd = &g_hd_channel[1].hd[0];

                hd->dev_no = i;
                hd->is_master = 1;
                hd->channel = &g_hd_channel[1];

                break;
            case 3:
                hd = &g_hd_channel[1].hd[1];

                hd->dev_no = i;
                hd->is_master = 0;
                hd->channel = &g_hd_channel[1];

                break;
        }

        if (i == 1) {
            g_active_hd = hd;
        }
    }
}

void hd_init() {
    printk("hd init...\n");

    _ide_channel_init();
    _hd_init();
}

void init_active_hd_info(u8 dev) {
    buffer_head_t *bh = kmalloc(sizeof(buffer_head_t));

    bh->data = kmalloc(512);
    bh->dev = dev;
    bh->sector_from = 0;
    bh->sector_count = 1;

    ll_rw_block(CHECK, bh);

    // 中断例程唤醒后才会执行
    // 从identify返回结果中取出硬盘信息
    memcpy(g_active_hd->number, bh->data + 10 * 2, 10 * 2);
    g_active_hd->number[20] = '\0';

    memcpy(g_active_hd->model, bh->data + 2 * 27, 2 * 20);
    g_active_hd->model[40] = '\0';

    g_active_hd->sectors = *(int *) (bh->data + 60 * 2);

    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));
}

void init_active_hd_partition() {
    // 读取hdb盘的MBR扇区
    buffer_head_t *buff = bread(g_active_hd->dev_no, 0, 1);

    // 赋值硬盘分区信息
    mbr_sector_t *mbr_sector = buff->data;
    memcpy(g_active_hd->partition, mbr_sector->partition, 64);

    kfree_s(buff->data, 512);
    kfree_s(buff, sizeof(buff));
}