#include "../../include/linux/hd.h"
#include "../../include/linux/fs.h"
#include "../../include/linux/mm.h"
#include "../../include/linux/kernel.h"
#include "../../include/linux/bitmap.h"
#include "../../include/string.h"
#include "../../include/assert.h"
#include "../../include/asm/io.h"

// 不考虑扩展分区
#define HD_PARTITION_MAX    4

#define SUPER_BLOCK_MAX     16

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

// 所有硬盘的所有分区都存这里
super_block_t g_super_block[SUPER_BLOCK_MAX];

// 活动分区（挂载分区、卸载分区，操作的就是这个
super_block_t *g_active_super_block;

// 空闲块位图
char block_bitmap_buf[512] = {0};
bitmap_t block_bitmap;

// inode节点位图
char inode_bitmap_buf[512] = {0};
bitmap_t inode_bitmap;

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

static super_block_t *find_empty_super_block() {
    for (int i = 0; i < SUPER_BLOCK_MAX; ++i) {
        if (g_super_block[i].type == 0) {
            return &g_super_block[i];
        }
    }

    panic("No superblock is available");
}

/**
 * 载入每个分区的超级块
 *
 * @param buff 第一个分区的超级块信息
 */
static void load_super_block(buffer_head_t *buff) {
    printk("The superblock has been initialized. loading...\n");

    for (int i = 0; i < HD_PARTITION_MAX; ++i) {
        hd_partition_t *partition = &g_active_hd->partition[i];

        if (partition->start_sect == 0) {
            printk("[load super block]hd partition %d is null! pass..\n", i);
            continue;
        }

        // 第一个分区的超级块已经传进来了，不用重复读硬盘获取
        if (i != 0) {
            buff = bread(g_active_hd->dev_no, g_active_hd->partition[i].start_sect + 1, 1);
        }

        // 将每个分区的超级块信息保存到内存中
        memcpy(find_empty_super_block(), buff->data, 512);

        if (i != 0) {
            kfree_s(buff->data, 512);
            kfree_s(buff, sizeof(buffer_head_t));
        }

        printk("[load super block]hd: %d, partition: %d, completed..\n", g_active_hd->dev_no, i);

        // 挂载分区
        if (i == 0) {
            mount_partition(&g_super_block[0]);

            // 初始化空闲块位图
            printk("[init block bitmap]read block bitmap sector: %d\n", g_super_block->block_bitmap_lba);

            buff = bread(g_active_hd->dev_no, g_super_block->block_bitmap_lba, 1);

            memcpy(&block_bitmap_buf, buff->data, 512);

            bitmap_make(&block_bitmap, block_bitmap_buf, 512, 0);

            kfree_s(buff->data, 512);
            kfree_s(buff, sizeof(buffer_head_t));

            // 初始化inode结点位图
            printk("[init inode bitmap]read inode bitmap sector: %d\n", g_super_block->inode_bitmap_lba);

            buff = bread(g_active_hd->dev_no, g_super_block->inode_bitmap_lba, 1);

            memcpy(&inode_bitmap_buf, buff->data, 512);
            bitmap_make(&inode_bitmap, inode_bitmap_buf, 512, 0);

            kfree_s(buff->data, 512);
            kfree_s(buff, sizeof(buffer_head_t));
        }
    }
}

static bool check_super_block_is_init() {
    bool ret = false;

    // 先检测超级块有没有创建，如果已经创建了，载入进来（注：超级块是存放在第1个扇区中的。第0扇区按照约定存放OS引导代码)
    buffer_head_t *buff = bread(g_active_hd->dev_no, g_active_hd->partition[0].start_sect + 1, 1);

    // 只要验证第一个分区的第一个字节即可（1、所有分区中的超级块都是一起创建的
    // 2、超级块的第一个字节是文件系统类型，至少为EXT=1
    if (*(uchar *) buff->data != 0) {
        ret = true;

        load_super_block(buff);
    }

    kfree_s(buff->data, 512);
    kfree_s(buff, sizeof(buffer_head_t));

    return ret;
}

void init_super_block() {
    assert(g_active_hd != NULL);

    printk("===== start: init super block =====\n");

    // 检测超级块是否创建
    if (check_super_block_is_init()) {
        return;
    }

    for (int i = 0; i < HD_PARTITION_MAX; ++i) {
        hd_partition_t *partition = &g_active_hd->partition[i];

        if (partition->start_sect == 0) {
            printk("[init super block]hd partition %d is null! pass..\n", i);

            continue;
        }

        super_block_t *super_block = find_empty_super_block();
        super_block->type = EXT;
        super_block->lba_base = partition->start_sect;
        super_block->sector_count = partition->nr_sects;

        // 计算存放空闲块位图需要几个扇区（一个扇区512B，可以映射512 × 8 = 4096个扇区
        int save_free_sector_bitmap_sector = partition->nr_sects / (512 * 8);
        save_free_sector_bitmap_sector += (partition->nr_sects % (512 * 8) == 0) ? 0 : 1;

        // 这个值是算出来的。当然只是一个估算值，在创建inode节点的时候还是要判断有没有扇区可用
        // 第一个+1表示第一个扇区固定用于存放OS启动代码
        // 第二个+1表示存放超级块数据
        super_block->block_bitmap_sects = save_free_sector_bitmap_sector;
        super_block->block_bitmap_lba = partition->start_sect + 1 + 1;

        super_block->inode_count = 4096;
        super_block->inode_bitmap_lba = super_block->block_bitmap_lba + super_block->block_bitmap_sects;
        super_block->inode_bitmap_sects = 1;

        // inode数组
        super_block->inode_table_lba = super_block->inode_bitmap_lba + super_block->inode_bitmap_sects;

        // 计算存放inode数组需要多少扇区
        super_block->inode_table_sects = super_block->inode_count * sizeof(d_inode_t) / 512;
        super_block->inode_table_sects += (super_block->inode_count * sizeof(d_inode_t) % 512 == 0) ? 0 : 1;

        super_block->root_lba = super_block->inode_table_lba + super_block->inode_table_sects;

        // 数据区开始的扇区
        super_block->data_start_lba = super_block->root_lba + 1;

        if (i == 0) {
            mount_partition(super_block);

            // 初始化空闲块位图
            bitmap_init(&block_bitmap, block_bitmap_buf, 512, 0);

            // 初始化inode结点位图
            bitmap_init(&inode_bitmap, inode_bitmap_buf, 512, 0);
        }

        // 将分区的超级块信息写入硬盘
        size_t write_size = bwrite(g_active_hd->dev_no, partition->start_sect + 1, (char *) super_block, 512);
        if (write_size == -1) {
            panic("save super block fail");
        } else {
            printk("save super block success: dev: %d, partition index: %d\n", g_active_hd->dev_no, i);
        }
    }

    printk("===== end: init super block =====\n");
}

void mount_partition(super_block_t *block) {
    assert(block != NULL);

    if (g_active_super_block != NULL) {
        panic("Please uninstall and mount it");
    }

    g_active_super_block = block;
}

void unmount_partition() {
    g_active_super_block = NULL;
}

void print_super_block() {
    assert(g_active_super_block != NULL);

    printk("lba base: %d\n", g_active_super_block->lba_base);
    printk("sector count: %d\n", g_active_super_block->sector_count);
    printk("os bootloader sector: %d\n", g_active_super_block->lba_base);
    printk("super block sector: %d\n", g_active_super_block->lba_base + 1);
    printk("block bitmap lba: %d\n", g_active_super_block->block_bitmap_lba);
    printk("block bitmap sectors: %d\n", g_active_super_block->block_bitmap_sects);
    printk("inode bitmap lba: %d\n", g_active_super_block->inode_bitmap_lba);
    printk("inode bitmap sectors: %d\n", g_active_super_block->inode_bitmap_sects);
    printk("inode table lba: %d\n", g_active_super_block->inode_table_lba);
    printk("inode table sectors: %d\n", g_active_super_block->inode_table_sects);
    printk("root lba: %d\n", g_active_super_block->root_lba);
    printk("data start lba: %d\n", g_active_super_block->data_start_lba);
}
