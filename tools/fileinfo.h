#ifndef OS_KERNEL_FILEINFO_H
#define OS_KERNEL_FILEINFO_H

typedef struct {
    char *name;
    int size;
    char *content;
} FileInfo;

FileInfo *read_file(const char *filename);

#endif