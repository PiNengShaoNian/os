#include "../include/linux/fs.h"
#include "../include/linux/hd.h"
#include "../include/linux/bitmap.h"
#include "../include/linux/kernel.h"
#include "../include/assert.h"

extern char inode_bitmap_buf[512];
extern bitmap_t inode_bitmap;

extern hd_t *g_active_hd;
extern super_block_t *g_active_super_block;

void iset(u32 index, bool v) {
    bitmap_set(&inode_bitmap, index, v);

    // 算下当前bitmap数据在哪个扇区
    int bitmap_sector = g_active_super_block->inode_bitmap_lba;

    printk("[save inode bitmap]sector: %d\n", bitmap_sector);

    int write_size = bwrite(g_active_hd->dev_no, bitmap_sector, inode_bitmap_buf, 512);
    assert(write_size != -1);
}

int iget() {
    int index = bitmap_scan(&inode_bitmap, 1);

    bitmap_set(&inode_bitmap, index, 1);

    // 算下当前bitmap数据在哪个扇区
    int bitmap_sector = g_active_super_block->inode_bitmap_lba;

    printk("[save inode bitmap]sector: %d\n", bitmap_sector);

    // 写回硬盘
    int write_size = bwrite(g_active_hd->dev_no, bitmap_sector, inode_bitmap_buf, 512);
    assert(write_size != -1);

    return index;
}
