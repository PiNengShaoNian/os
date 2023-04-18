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
} __attribute__((packed)) buffer_head_t;

typedef struct _hd_request_t {
    int dev;                        /* -1 if no request */
    int cmd;                        /* READ or WRITE */
    int errors;
    unsigned long sector;           // 从哪个扇区开始读
    unsigned long nr_sectors;       // 读多少扇区
    char *buffer;
    struct buffer_head_t *bh;
    struct request *next;
} __attribute__((packed)) hd_request_t;

void ll_rw_block(int rw, buffer_head_t *bh);

/**
 * 发送读硬盘请求
 * @param dev   读哪块盘
 * @param from  从哪个扇区开始读
 * @param count 读多少扇区
 * @return
 */
buffer_head_t *bread(int dev, int from, int count);

size_t bwrite(int dev, int block, buffer_head_t *bh);

#endif // OS_FS_H