#include "floppy.h"
#include "common.h"

Floppy *create_floppy() {
    Floppy *floppy = calloc(1, sizeof (Floppy));

//    * @param face 哪个面: 0 1
//    * @param track 哪个磁道: 0-79
//    * @param section 哪个扇区: 1-18
    floppy->size = 2 * 80 * 18 * 512;
    floppy->content = calloc(1, floppy->size);

    return  floppy;
}

void write_bootloader(Floppy *floppy, FileInfo *fileInfo) {
    if(floppy == NULL || fileInfo == NULL) {
        ERROR_PRINT("NULL pointer\n");
        return;
    }

    memcpy(floppy->content, fileInfo->content, fileInfo->size);
}

void write_floppy_fileinfo(Floppy *floppy, FileInfo *fileInfo, int face, int trace, int section) {
    if(floppy == NULL || fileInfo == NULL){
        ERROR_PRINT("NULL pointer\n");
        return;
    }

    char *offset = 0;

    printf("[%s] 文件大小超过 512: %d\n", fileInfo->name, fileInfo->size);

    offset = floppy->content + (section - 1) * 512;

    memcpy(offset, fileInfo->content, fileInfo->size);
}

void create_image(const char *name, Floppy *floppy) {
    if(name == NULL || floppy == NULL) {
        ERROR_PRINT("NULL pointer\n");
        return;
    }

    FILE *file = fopen(name, "w+");
    if(file == NULL) {
        perror("fopen fail: ");
        exit(-1);
    }

    fwrite(floppy->content, 1, floppy->size, file);
}