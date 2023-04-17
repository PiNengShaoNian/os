#ifndef OS_IDE_H
#define OS_IDE_H

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

void hd_init();

#endif // OS_IDE_H