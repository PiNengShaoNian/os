#include "../../include/linux/hd.h"
#include "../../include/linux/fs.h"
#include "../../include/linux/mm.h"
#include "../../include/linux/kernel.h"
#include "../../include/linux/bitmap.h"
#include "../../include/linux/task.h"
#include "../../include/string.h"
#include "../../include/assert.h"
#include "../../include/shell.h"
#include "../../include/asm/io.h"

extern task_t *current;

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
    mbr_sector_t *mbr_sector = (mbr_sector_t *) buff->data;
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

//            buff = bread(g_active_hd->dev_no, g_super_block->block_bitmap_lba, 1);
//
//            memcpy(&block_bitmap_buf, buff->data, 512);

            memset(&block_bitmap_buf, 0, 512);

            bitmap_make(&block_bitmap, block_bitmap_buf, 512, 0);

//            kfree_s(buff->data, 512);
//            kfree_s(buff, sizeof(buffer_head_t));

            // 初始化inode结点位图
            printk("[init inode bitmap]read inode bitmap sector: %d\n", g_super_block->inode_bitmap_lba);

//            buff = bread(g_active_hd->dev_no, g_super_block->inode_bitmap_lba, 1);
//
//            memcpy(&inode_bitmap_buf, buff->data, 512);
            memset(&inode_bitmap_buf, 0, 512);
            bitmap_make(&inode_bitmap, inode_bitmap_buf, 512, 0);

//            kfree_s(buff->data, 512);
//            kfree_s(buff, sizeof(buffer_head_t));
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

void print_block_bitmap() {
    printk("print block bitmap\n");

    assert(g_active_super_block != NULL);

    printk("block bitmap lba: %d\n", g_active_super_block->block_bitmap_lba);
    printk("block bitmap sectors: %d\n", g_active_super_block->block_bitmap_sects);

    printk("[mm]block bitmap: ");
    for (int i = 0; i < 10; ++i) {
        printk("0x%02x ", block_bitmap_buf[i]);
    }
    printk("\n");

    buffer_head_t *bh = bread(g_active_hd->dev_no, g_active_super_block->block_bitmap_lba, 1);
    printk("[hd]block bitmap: ");
    for (int i = 0; i < 10; ++i) {
        printk("0x%02x ", bh->data[i]);
    }
    printk("\n");

    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));
}

void reset_block_bitmap() {
    printk("reset block bitmap\n");

    // 将内存中的全置为0
    memset(block_bitmap_buf, 0, 512);

    // 根目录数据区占用数据区第一个扇区
//    block_bitmap_buf[0] = 1;

    // 写入硬盘
    int write_size = bwrite(g_active_hd->dev_no, g_active_super_block->block_bitmap_lba, block_bitmap_buf, 512);
    assert(write_size != -1);
}

void print_inode_bitmap() {
    printk("print inode bitmap\n");

    assert(g_active_super_block != NULL);

    printk("inode bitmap lba: %d\n", g_active_super_block->inode_bitmap_lba);
    printk("inode bitmap sectors: %d\n", g_active_super_block->inode_bitmap_sects);

    printk("[mm]inode bitmap: ");
    for (int i = 0; i < 10; ++i) {
        printk("0x%02x ", inode_bitmap_buf[i]);
    }
    printk("\n");

    buffer_head_t *bh = bread(g_active_hd->dev_no, g_active_super_block->inode_bitmap_lba, 1);
    printk("[hd]inode bitmap: ");
    for (int i = 0; i < 10; ++i) {
        printk("0x%02x ", bh->data[i]);
    }
    printk("\n");

    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));
}

void reset_inode_bitmap() {
    printk("reset inode bitmap\n");

    // 将内存中的全置为0
    memset(inode_bitmap_buf, 0, 512);

    // inode表第一个数据项是根目录的
//    inode_bitmap_buf[0] = 1;

    // 写入硬盘
    int write_size = bwrite(g_active_hd->dev_no, g_active_super_block->inode_bitmap_lba, inode_bitmap_buf, 512);
    assert(write_size != -1);
}

void print_bitmap() {
    print_block_bitmap();
    print_inode_bitmap();
}

void reset_bitmap() {
    reset_block_bitmap();
    reset_inode_bitmap();
}

void create_root_dir() {
    printk("===== start: create root dir =====\n");

    int write_size;
    char *name = "/";

    // 1. 创建目录项
    dir_entry_t *dir_entry = kmalloc(512);

    memset(dir_entry->name, 0, 16);
    memcpy(dir_entry->name, name, strlen(name));

    dir_entry->ft = FILE_TYPE_DIRECTORY;
    dir_entry->dir_index = 0;

    // 2. 申请inode index (第一次写硬盘
    dir_entry->inode = iget();

    // 3. 将目录项写入硬盘 (第二次写硬盘
    write_size = bwrite(g_active_hd->dev_no, g_active_super_block->root_lba, (char *) dir_entry, 512);
    assert(write_size != -1);

    printk("[save root directory entry]sector: %d\n", g_active_super_block->root_lba);

    /* 存储inode */
    // 先计算inode存储在哪个扇区
    int inode_sector = g_active_super_block->inode_table_lba;

    printk("[save inode]inode sector: %d\n", inode_sector);

    // 创建inode对象 (复用dir_entry申请的内存
    memset(dir_entry, 0, 512);

    d_inode_t *inode = (d_inode_t *) dir_entry;
    inode->i_mode = 777;
    inode->i_size = 0;
    inode->i_zone_off = 0;

    // 申请数据块 (第三次写硬盘
    inode->i_zone[inode->i_zone_off++] = get_data_sector();
    printk("zone: %d, off: %d\n", inode->i_zone[0], inode->i_zone_off);

    // 将inode对象写入硬盘 (第四次写硬盘
    write_size = bwrite(g_active_hd->dev_no, inode_sector, (char *) inode, 512);
    assert(write_size != -1);

    kfree_s(dir_entry, 512);

    printk("===== end: create root dir =====\n");
}

void print_root_dir() {
    assert(current != NULL);
    assert(current->current_active_dir != NULL);

    printk("[mm]current work dir: %s\n", current->current_active_dir->name);
    printk("[mm]current work dir inode: %d\n", current->current_active_dir->inode);
    printk("[mm]current work dir data index: %d\n", current->current_active_dir->dir_index);

    printk("[mm]current work dir data zone: ");
    for (int i = 0; i < current->current_active_dir_inode->i_zone_off; ++i) {
        printk("%d ", current->current_active_dir_inode->i_zone[i]);
    }
    printk("\n");

    buffer_head_t *bh = bread(g_active_hd->dev_no, g_active_super_block->root_lba, 1);
    dir_entry_t *dir = (dir_entry_t *) bh->data;

    printk("[hd]current work dir: %s\n", dir->name);
    printk("[hd]current work dir inode: %d\n", dir->inode);
    printk("[hd]current work dir data index: %d\n", dir->dir_index);
    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));

    bh = bread(g_active_hd->dev_no, g_active_super_block->inode_table_lba, 1);
    m_inode_t *inode = (m_inode_t *) bh->data;

    printk("[hd]current work dir data zone: ");
    for (int i = 0; i < inode->i_zone_off; ++i) {
        printk("%d ", inode->i_zone[i]);
    }
    printk("\n");

    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));
}

void ls_current_dir() {
    // 拿到根目录数据
    dir_entry_t *entry = NULL;
    buffer_head_t *bh = NULL;
    u32 dir_inode = 0;
    u32 dir_index = 0;

    if (!strcmp(current->current_active_dir->name, "/")) {
        entry = current->current_active_dir;
        dir_inode = entry->inode;
        dir_index = entry->dir_index;
    } else {
        bh = bread(g_active_hd->dev_no, g_active_super_block->root_lba, 1);
        entry = bh->data;
        dir_inode = entry->inode;
        dir_index = entry->dir_index;

        kfree_s(bh->data, 512);
        kfree_s(bh, sizeof(buffer_head_t));
    }

    printk("inode index: %d\n", dir_inode);
    printk("dir index: %d\n", dir_index);

    if (dir_index == 0) {
        printk("empty!\n");
        return;
    }

    // 拿到根目录的inode
    bh = bread(g_active_hd->dev_no, g_active_super_block->inode_table_lba, 1);
    m_inode_t *inode = bh->data + dir_inode * sizeof(m_inode_t);

    // 拿到根目录存储数据的扇区号
    int zone = inode->i_zone[inode->i_zone_off - 1];
    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));

    printk("data zone: %d\n", zone);

    // 读硬盘拿到根目录中所有的目录项
    bh = bread(g_active_hd->dev_no, zone, 1);

    entry = bh->data;
    for (int i = 0; i < dir_index; ++i) {
        printk("%s ", entry->name);

        entry++;
    }
    printk("\n");

    // 释放内存
    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(bh));
}

/**
 * 创建目录是很复杂的过程，需要与硬盘交互很多次，顺序可以随意，没有哪个要先做哪个要后做
 * 但是必须保证整个流程是原子操作，不能被打断，否则硬盘的数据就是乱的
 * @param name
 */
void create_dir(char *name) {
    int write_size, inode_index;

    int parent_inode_current_zone = current->current_active_dir_inode->i_zone[
            current->current_active_dir_inode->i_zone_off - 1];
    assert(parent_inode_current_zone >= 0);

    printk("[%s]parent_inode_current_zone: %d\n", __FUNCTION__, parent_inode_current_zone);

    // 读出存储目录项的那个扇区
    buffer_head_t *bh = bread(g_active_hd->dev_no, parent_inode_current_zone, 1);
    printk("[%s]create %d directory\n", __FUNCTION__, current->current_active_dir->dir_index);

    // 1. 创建目录项
    dir_entry_t *dir_entry = bh->data + sizeof(dir_entry_t) * current->current_active_dir->dir_index++;

    memset(dir_entry->name, 0, 16);
    memcpy(dir_entry->name, name, strlen(name));

    dir_entry->ft = FILE_TYPE_DIRECTORY;
    dir_entry->dir_index = 0;

    // 2. 申请inode index (第一次写硬盘
    inode_index = dir_entry->inode = iget();
    print_inode_bitmap();

    // 3. 将目录的目录项写入硬盘 (第二次写硬盘
    write_size = bwrite(g_active_hd->dev_no, parent_inode_current_zone, bh->data, 512);
    assert(write_size != -1);

    printk("[save directory entry]sector: %d\n", parent_inode_current_zone);

    // 父目录的属性有更新，写回去
    memset(dir_entry, 0, sizeof(dir_entry_t));
    memcpy(dir_entry, current->current_active_dir, sizeof(dir_entry));

    printk("[write parent dir info]dir index:%d\n", current->current_active_dir->dir_index);

    write_size = bwrite(g_active_hd->dev_no, g_active_super_block->root_lba, dir_entry, 512);
    assert(write_size != -1);

    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));

    // 存储inode
    int inode_sector = g_active_super_block->inode_table_lba;
    printk("[save inode]inode sector: %d\n", inode_sector);
    bh = bread(g_active_hd->dev_no, inode_sector, 1);

    // TODO inode数组数据需要从硬盘中读，不然永远写的都是第一个
    d_inode_t *inode = bh->data + inode_index * sizeof(d_inode_t);
    inode->i_mode = 777;
    inode->i_size = 0;
    inode->i_zone_off = 0;

    // 申请数据块（第三次写硬盘
    inode->i_zone[inode->i_zone_off++] = get_data_sector();
    print_block_bitmap();

    // 将inode对象写入硬盘（第四次写硬盘
    write_size = bwrite(g_active_hd->dev_no, inode_sector, bh->data, 512);
    assert(write_size != -1);

    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));
}

static get_filepath_depth(const char *filepath) {
    assert(filepath != NULL);

    int ret = 0;

    char *tmp = filepath;
    char ch;
    while ('\0' != (ch = *tmp++)) {
        if ('/' == ch) ret++;
    }

    return ret;
}

static filepath_parse_result *parse_filepath(const char *filepath) {
    assert(filepath != NULL);

    filepath_parse_result *ret = kmalloc(sizeof(filepath_parse_result));

    ret->depth = get_filepath_depth(filepath);
    ret->data = kmalloc(sizeof(char *) * ret->depth);

    // 解析出每个/所在的位置
    char ch;
    int ch_off = 0;
    int ret_off = 0;
    while ((ch = filepath[ch_off]) != '\0') {
        if (ch == '/') {
            ret->data[ret_off] = (char *) ch_off;
            ret_off++;
        }

        ch_off += 1;
    }

    // 将/之间的数据取出来，索引顺序即目录父子关系
    for (int i = 0; i < ret->depth; ++i) {
        // 目录名从哪开始
        int index = (int) ret->data[i];

        // 目录名到哪（如果是最后一个/就到结尾
        int index_next = (i != ret->depth - 1) ? (int) ret->data[i + 1] : strlen(filepath);

        // 目录名称不能超过16
        assert(index_next - index - 1 < 16);

        // 这里为什么要申请16
        ret->data[i] = kmalloc(16);
        memset(ret->data[i], 0, 16);
        memcpy(ret->data[i], filepath + index + 1, index_next - index - 1);
    }

    return ret;
}

/**
 * 获取一个目录的所有目录项（只实现读第一个数据区
 * @param dir_name
 * @return
 */
static dir_entry_t *get_root_directory_children() {
    // 拿到根目录存储数据的扇区号
    int zone = current->root_dir_inode->i_zone[current->root_dir_inode->i_zone_off - 1];

    // 读硬盘拿到根目录中所有的目录项
    buffer_head_t *bh = bread(g_active_hd->dev_no, zone, 1);

    dir_entry_t *ret = (dir_entry_t *) bh->data;

    kfree_s(bh, sizeof(bh));

    return ret;
}

void rm_directory(const char *filepath) {
    assert(filepath != NULL);

    int write_size = 0;
    char *buf = kmalloc(512);

    filepath_parse_result *parse_result = parse_filepath(filepath);
    if (parse_result->depth == 0) {
        printk("path depth less than 1!\n");
        return;
    }

    dir_entry_t *children = get_root_directory_children();
    if (children->name[0] == 0) {
        printk("empty!\n");
        goto done;
    }

    // 判断目录是否存在
    dir_entry_t *entry = NULL;
    dir_entry_t *tmp = (dir_entry_t *) children;
    while (tmp != NULL && (tmp->name[0] != 0)) {
        if (!strcmp(parse_result->data[0], tmp->name)) {
            entry = tmp;
            break;
        }

        tmp++;
    }

    if (entry == NULL) {
        printk("directory %s not exists", parse_result->data[0]);
        goto done;
    }

    // 判断是不是目录
    if (entry->ft != FILE_TYPE_DIRECTORY) {
        printk("not a directory\n");
        goto done;
    }

    print_dir_entry(entry);

    current->current_active_dir->dir_index--;
    memcpy(buf, current->current_active_dir, sizeof(dir_entry_t));

    printk("[write parent dir info]dir index:%d\n", current->current_active_dir->dir_index);

    write_size = bwrite(g_active_hd->dev_no, g_active_super_block->root_lba, buf, 512);
    assert(write_size != -1);

    // 释放inode位图
    iset(entry->inode, 0);

    // 释放block位图
    buffer_head_t *bh = bread(g_active_hd->dev_no, g_active_super_block->inode_table_lba, 1);
    m_inode_t *inode = bh->data + entry->inode * sizeof(d_inode_t);

    set_data_sector(inode->i_zone[0], 0);

    kfree_s(bh->data, 512);
    kfree_s(bh, sizeof(buffer_head_t));

    done:
    kfree_s(buf, 512);
    kfree_s(children, 512);
}


void print_dir_entry(dir_entry_t *entry) {
    assert(entry != NULL);

    printk("name: %s\n", entry->name);
    printk("file type: %d\n", entry->ft);
    printk("inode: %d\n", entry->inode);
    printk("dir index: %d\n", entry->dir_index);
}
