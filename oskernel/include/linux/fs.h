#ifndef OS_FS_H
#define OS_FS_H

#include "types.h"

#define READ   0
#define WRITE  1
#define READA  2        /* read-ahead - don't pause */
#define WRITEA 3    /* "write-ahead" - silly, but somewhat useful */
#define CHECK  4     // 检测硬盘,获取硬盘容量等信息

typedef struct _buffer_head_t {
    char *data;
    u8 dev;            // 读哪块硬盘
    uint sector_from;    // 从哪个扇区开始读
    uint sector_count;   // 读几个扇区
    u8 handler_state;       // 读写硬盘的结果
} __attribute__((packed)) buffer_head_t;

typedef struct _hd_request_t {
    int dev;                        /* -1 if no request */
    int cmd;                        /* READ or WRITE */
    int errors;
    unsigned long sector;           // 从哪个扇区开始读
    unsigned long nr_sectors;       // 读多少扇区
    char *buffer;
    struct _buffer_head_t *bh;
    struct _hd_request_t *next;
} __attribute__((packed)) hd_request_t;

typedef struct _d_inode_t {
    unsigned short i_mode;  // 文件类型和属性(rwx)
    unsigned short i_uid;   // 所属用户的uid
    unsigned long i_size;   // 文件大小(单位byte)
    unsigned long i_time;   // 修改时间
    unsigned char i_gid;    // 所属用户的gid
    unsigned char i_nlinks; // 链接数(多少文件目录项指向该inode)
    unsigned short i_zone[9];   // 直接0-6, 一级7, 二级8
    u8 i_zone_off;
} __attribute__((packed)) d_inode_t;

void ll_rw_block(int rw, buffer_head_t *bh);

/**
 * 发送读硬盘请求
 * @param dev   读哪块盘
 * @param from  从哪个扇区开始读
 * @param count 读多少扇区
 * @return
 */
buffer_head_t *bread(int dev, int from, int count);

size_t bwrite(int dev, int from, char *buff, int size);

int iget();

void create_root_dir();

#endif // OS_FS_H