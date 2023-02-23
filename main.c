#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include "tools/floppy.h"
#include "tools/fileinfo.h"

/**
 * 获得该可执行文件所处的绝对路径
 * @param path
 */
void get_exec_path(char  * const path) {
    int count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count == -1) {
        perror("readlink");
        exit(EXIT_FAILURE);
    }
    path[count] = '\0';
}

int main(int argc, char **argv) {
    char exec_path[PATH_MAX];
    get_exec_path(exec_path);
    // 获取工作目录的在path中的起始位置
    char *p = strstr(exec_path, "/os/");
    // 忽略后续的的路径
    p[4] = '\0';
    char *project_path = exec_path;
    Floppy *floppy = create_floppy();

    // boot文件的路径
    char boot_file_path[PATH_MAX];
    sprintf(boot_file_path, "%s%s",project_path, "build/boot/boot.o");

    FileInfo *boot_fileinfo = read_file(boot_file_path);
    write_bootloader(floppy, boot_fileinfo);

    char setup_file_path[PATH_MAX];
    sprintf(setup_file_path, "%s%s", project_path, "build/boot/setup.o");
    FileInfo *setup_fileinfo = read_file(setup_file_path);
    write_floppy_fileinfo(floppy, setup_fileinfo, 0, 0, 2);

    // 软盘的路径
    char img_filename[PATH_MAX];
    sprintf(img_filename, "%sa.img", project_path);
    create_image(img_filename, floppy);
    return 0;
}
