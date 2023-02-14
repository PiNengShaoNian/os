#include "fileinfo.h"
#include "common.h"

FileInfo *read_file(const char *filename) {
    if (filename == NULL)
        return NULL;

    // 1 创建对象
    FileInfo *fileinfo = calloc(1, sizeof(FileInfo));
    if (fileinfo == NULL) {
        perror("calloc fail: ");
        exit(-1);
    }

    fileinfo->name = filename;

    // 2 打开文件
    FILE *file = NULL;
    if ((file = fopen(filename, "rb")) == NULL) {
        perror("fopen fail");
        exit(1);
    }

    // 3获取文件大小
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("fseek fail");
        exit(1);
    }

    fileinfo->size = (int) ftell(file);
    if (fileinfo->size == -1) {
        perror("ftell fail");
        exit(1);
    }

    // 文件指针还原
    fseek(file, 0, SEEK_SET);

    // 4 申请内存
    fileinfo->content = calloc(1, fileinfo->size);
    if (fileinfo->content == NULL) {
        perror("calloc fail: ");
        exit(-1);
    }

    // 5 文件读入
    int readsize = fread(fileinfo->content, sizeof(char), fileinfo->size, file);
    if (readsize != fileinfo->size) {
        perror(('fread fail: '));
        exit(-1);
    }

    // 6 关闭文件
    fclose(file);

    return fileinfo;
}