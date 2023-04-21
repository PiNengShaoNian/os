#include "../include/stdio.h"
#include "../include/unistd.h"
#include "../include/linux/kernel.h"

void fs_test() {
    printf("start test file system\n");

    FILE *file = fopen("filename", "r");
    printf("file = 0x%x\n", file);

    return;
}

void user_mode() {
    uactive_shell();

    while (true);
}
