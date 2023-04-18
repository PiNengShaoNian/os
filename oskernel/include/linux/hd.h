#ifndef OS_IDE_H
#define OS_IDE_H

#include "./kernel.h"

/* Hd controller regs. Ref: IBM AT Bios-listing */
#define HD_DATA        0x1f0    /* _CTL when writing */
#define HD_ERROR       0x1f1    /* see err-bits */
#define HD_NSECTOR     0x1f2    /* nr of sectors to read/write */
#define HD_SECTOR      0x1f3    /* starting sector */
#define HD_LCYL        0x1f4    /* starting cylinder */
#define HD_HCYL        0x1f5    /* high byte of starting cyl */
#define HD_CURRENT     0x1f6    /* 101dhhhh , d=drive, hhhh=head */
#define HD_STATUS      0x1f7    /* see status-bits */
#define HD_PRECOMP     HD_ERROR    /* same io address, read=error, write=precomp */
#define HD_COMMAND     HD_STATUS    /* same io address, read=status, write=cmd */

#define HD_CMD         0x3f6

/* Bits of HD_STATUS */
#define ERR_STAT      0x01    // 1表示发生了错误,错误代码已放置在错误寄存器中
#define INDEX_STAT    0x02    // 1表示控制器检测到索引标记(啥意思?)
#define ECC_STAT      0x04    // 1 表示控制器必须通过使用 ECC 字节来纠正数据（纠错码：扇区末尾的额外字节，允许验证其完整性，有时还可以纠正错误）
#define DRQ_STAT      0x08    // 1 表示控制器正在等待数据（用于写入）或正在发送数据（用于读取）。该位为 0 时不要访问数据寄存器。
#define SEEK_STAT     0x10    // 1 表示读/写磁头就位（搜索完成）
#define WRERR_STAT    0x20    // 1 表示控制器检测到写入故障
#define READY_STAT    0x40    // 1 表示控制器已准备好接受命令，并且驱动器以正确的速度旋转
#define BUSY_STAT     0x80    // 1 表示控制器正忙于执行命令。设置该位时，不应访问任何寄存器（数字输出寄存器除外）

/* Values for HD_COMMAND */
#define WIN_RESTORE        0x10
#define WIN_READ           0x20
#define WIN_WRITE          0x30
#define WIN_VERIFY         0x40
#define WIN_FORMAT         0x50
#define WIN_INIT           0x60
#define WIN_SEEK           0x70
#define WIN_DIAGNOSE       0x90
#define WIN_SPECIFY        0x91

/* Bits for HD_ERROR */
#define MARK_ERR          0x01    /* Bad address mark ? */
#define TRK0_ERR          0x02    /* couldn't find track 0 */
#define ABRT_ERR          0x04    /* ? */
#define ID_ERR            0x10    /* ? */
#define ECC_ERR           0x40    /* ? */
#define    BBD_ERR        0x80    /* ? */

// insw指令从DX寄存器中指定的端口读取一个字（16位），并将其复制到ES:DI指向的内存地址中。然后，DI寄存器自动增加2，CX寄存器减1。如果CX寄存器的值为0，则输入字符串操作结束。
// 需要注意的是，insw指令需要使用rep指令结合使用，以便可以重复执行输入操作。通常情况下，insw指令用于读取设备I/O端口中的数据，并将其存储在内存中。
// 另外，insw指令需要确保指定的端口中存在有效数据。如果端口中没有有效数据，则会发生阻塞等待的情况，这可能会导致程序陷入死循环。因此，在使用insw指令之前，需要确保端口中存在有效数据。
#define port_read(port, buf, nr) \
__asm__("cld;rep;insw"::"d" (port),"D" (buf),"c" (nr))

#define port_write(port, buf, nr) \
__asm__("cld;rep;outsw"::"d" (port),"S" (buf),"c" (nr))

typedef struct _hd_partition_t {

} __attribute__((packed)) hd_partition_t;

typedef struct _hd_channel_t hd_channel_t;

typedef struct _hd_t {
    u8 dev_no;
    u8 is_master;      // 是否是主设备 1是 0否
    hd_partition_t partition[4];   // 暂不考虑逻辑分区
    hd_channel_t *channel;

    // 数据来源：硬盘identify命令返回的结果
    char number[10 * 2 + 1];    // 硬盘序列号    最后一个字节是补字符串结束符0用的,从硬盘读取的是没有结束符的
    char model[20 * 2 + 1];     // 硬盘型号
    int sectors;                // 扇区数 一个扇区512字节
} __attribute__((packed)) hd_t;

typedef struct _hd_channel_t {
    hd_t hd[2];      // index=0为主设备，index=1为从设备
    u16 port_base;  // 通过哪个端口操作该通道 通道1对应的端口0x1f0-0x1f7,控制寄存器端口0x3f6.通道1对应的端口号0x170-0x177,控制寄存器端口0x376
    u8 irq_no;     // 该通道触发的中断由哪个中断程序处理
} __attribute__((packed)) hd_channel_t;

typedef void (*dev_handler_fun_t)(void);

void hd_init();

void do_hd_request();

hd_t *get_hd_info(u8 dev);

void print_disk_info(hd_t *info);

void init_active_hd_info(u8 dev);

#endif // OS_IDE_H