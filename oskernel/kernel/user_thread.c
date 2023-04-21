#include "../include/stdio.h"
#include "../include/unistd.h"
#include "../include/linux/kernel.h"

void fs_test() {
    printf("start test file system\n");

    FILE *file = fopen("test", "r");

    printf("file = 0x%x\n", file);

    if (file == NULL) {
        return;
    }

    char buff[64] = {0};
    int ret = fread(buff, 63, file);

    if (ret < 0) {
        printf("read failed\n");
        goto cleanup;
    }

    printf("content: %s\n", buff);

    cleanup:
    ret = fclose(file);

    if (ret == 0) {
        printf("close success!\n");
    }

    return;
}

void user_mode() {
    uactive_shell();

    while (true);
}
