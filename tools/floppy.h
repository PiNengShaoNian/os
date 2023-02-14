#ifndef OS_KERNEL_FLOPPY_H
#define OS_KERNEL_FLOPPY_H

#include "fileinfo.h"

typedef struct {
    int size;
    char *content;
} Floppy;

Floppy *create_floppy();

void write_bootloader(Floppy *floppy, FileInfo *fileInfo);

/**
 *
 * @param floppy
 * @param str
 * @param face 哪个面: 0 1
 * @param track 哪个磁道: 0-79
 * @param section 哪个扇区: 1-18
 */
void write_floppy(Floppy *floppy, char *str, int face, int track, int section);

void write_floppy_fileinfo(Floppy *floppy, FileInfo *fileinfo, int face, int track, int section);

/**
 * 生成内核软盘
 * @param name 内核文件名称
 * @param floppy 软盘对象
 */
void create_image(const char *name, Floppy *floppy);

#endif // OS_KERNEL_FLOPPY_H