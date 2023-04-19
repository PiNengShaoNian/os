#include "../include/linux/sys.h"
#include "../include/linux/kernel.h"
#include "../include/linux/task.h"
#include "../include/linux/fs.h"
#include "../include/linux/hd.h"
#include "../include/asm/system.h"
#include "../include/string.h"
#include "../include/assert.h"

extern task_t *current;
extern hd_t *g_active_hd;
extern super_block_t *g_active_super_block;

static int open_root_dir(int flags) {
    assert(current != NULL);

    int fd = find_empty_file_descriptor();

    // 拿到根目录的目录项
    buffer_head_t *bh = bread(g_active_hd->dev_no, g_active_super_block->root_lba, 1);
    current->root_dir = (dir_entry_t *) bh->data;
    kfree_s(bh, sizeof(buffer_head_t));

    printk("[%s]dir entry name: %s\n", __FUNCTION__, current->root_dir->name);

    // 拿到根目录的inode
    bh = bread(g_active_hd->dev_no, g_active_super_block->inode_table_lba, 1);
    current->root_dir_inode = (m_inode_t *) bh->data;
    kfree_s(bh, sizeof(buffer_head_t));

    // 创建文件描述符
    file_t *file = kmalloc(sizeof(file_t));
    file->f_mode = flags;
    file->f_flags = 0;
    file->f_count = fd;
    file->f_inode = current->root_dir_inode;
    file->f_pos = 0;

    current->file_descriptor[fd] = file;
    current->current_active_dir = current->root_dir;
    current->current_active_dir_inode = current->root_dir_inode;

    return fd;
}

int sys_open(const char *pathname, int flags) {
    int fd = -1;

    printk("===== start open %s =====\n", pathname);
    if (!strcmp("/", pathname)) {
        fd = open_root_dir(flags);
    } else {}

    printk("===== end open %s =====\n", pathname);

    return fd;
}