#ifndef OS_FS_H
#define OS_FS_H

#include "types.h"

#define READ   0
#define WRITE  1
#define READA  2        /* read-ahead - don't pause */
#define WRITEA 3    /* "write-ahead" - silly, but somewhat useful */
#define CHECK  4     // 检测硬盘,获取硬盘容量等信息

#define O_ACCMODE       0003
#define O_RDONLY         00
#define O_WRONLY         01
#define O_RDWR             02
#ifndef O_CREAT
# define O_CREAT       0100    /* Not fcntl.  */
#endif
#ifndef O_EXCL
# define O_EXCL           0200    /* Not fcntl.  */
#endif
#ifndef O_NOCTTY
# define O_NOCTTY       0400    /* Not fcntl.  */
#endif
#ifndef O_TRUNC
# define O_TRUNC      01000    /* Not fcntl.  */
#endif
#ifndef O_APPEND
# define O_APPEND      02000
#endif
#ifndef O_NONBLOCK
# define O_NONBLOCK      04000
#endif
#ifndef O_NDELAY
# define O_NDELAY    O_NONBLOCK
#endif
#ifndef O_SYNC
# define O_SYNC           04010000
#endif
#define O_FSYNC        O_SYNC
#ifndef O_ASYNC
# define O_ASYNC     020000
#endif
#ifndef __O_LARGEFILE
# define __O_LARGEFILE    0100000
#endif

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

typedef struct _m_inode_t {
    unsigned short i_mode;  // 文件类型和属性(rwx)
    unsigned short i_uid;   // 所属用户的uid
    unsigned long i_size;   // 文件大小(单位byte)
    unsigned long i_time;   // 修改时间
    unsigned char i_gid;    // 所属用户的gid
    unsigned char i_nlinks; // 链接数(多少文件目录项指向该inode)
    unsigned short i_zone[9];   // 直接0-6, 一级7, 二级8
    u8 i_zone_off;
} __attribute__((packed)) m_inode_t;

typedef enum {
    FILE_TYPE_REGULAR = 1, // 普通文件
    FILE_TYPE_DIRECTORY // 目录文件
} file_type;

typedef struct _dir_entry_t {
    char name[16];  // 根据第一个字节是不是0来判断根目录扇区是否初始化
    ushort inode;  // 该目录项对应的inode结点在inode数组中的index
    file_type ft;
    u32 dir_index;  // 如果是目录，下一个文件目录项的index
} __attribute__((packed)) dir_entry_t;

typedef struct _file_t {
    unsigned short f_mode; // 文件读写权限
    unsigned short f_flags; // 文件控制标志
    unsigned short f_count;// 对应文件句柄
    m_inode_t *f_inode; // 对应inode
    uint f_pos; // 文件偏移
} __attribute__((packed)) file_t;

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

void iset(u32 index, bool v);

int iget();

int get_data_sector();

void set_data_sector(u32 index, bool v);

void print_dir_entry(dir_entry_t *entry);

void create_root_dir();

void print_root_dir();

void ls_current_dir();

void create_dir(char *name);

void cd_directory(const char *filepath);

#endif // OS_FS_H