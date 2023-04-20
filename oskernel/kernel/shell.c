#include "../include/linux/kernel.h"
#include "../include/linux/hd.h"
#include "../include/linux/fs.h"
#include "../include/linux/mm.h"
#include "../include/linux/task.h"
#include "../include/shell.h"
#include "../include/string.h"
#include "../include/assert.h"

extern task_t *current;
bool g_active_shell = false;

// 用于存储键盘输入的shell命令
ushort g_shell_command_off = 0;
char g_shell_command[64] = {0};

bool shell_is_active() {
    return g_active_shell;
}

bool active_shell() {
    g_active_shell = true;

    printk("shell activated!\n");
    printk("[%s]> ", current->current_active_dir->name);
}

bool close_shell() {
    g_active_shell = false;
}

void put_char_shell(char ch) {
    if (!g_active_shell) return;

    if (g_shell_command_off >= 64) {
        panic("the command length exceed 64!\n");
    }

    g_shell_command[g_shell_command_off++] = ch;
}

/**
 * 解析用户输入的shell字符串有几个有限字符串（这些字符串就是命令 + 参数
 * @return
 */
static int shell_command_size() {
    int size = 0;

    int off = 0;
    char ch;
    while ((ch = g_shell_command[off++]) != '\0') {
        if (ch == ' ') {
            size++;

            // 后面接着的空格耗完
            while (g_shell_command[off] == ' ') {
                off++;
            }
        }
    }

    return size;
}

/**
 * 解析用户输入的shell命令
 * @param arr_len
 * @return
 */
static char **parse_shell_command(OUT int *arr_len) {
    char **ret = kmalloc(shell_command_size() * sizeof(char *));

    int size = 0;   // 碰到一个空格+1, 连续的空格不算
    int off = 0;    // 字符串游标
    int command_len = 0;    // 命令长度
    int command_start = 0;  // 命令或参数开始的位置
    char ch;

    while ((ch = g_shell_command[off++]) != '\0') {
        if (ch != ' ') {
            command_len++;
        }

        if (g_shell_command[off] == '\0') {
            char *str = kmalloc(command_len + 1);
            memset(str, 0, command_len + 1);

            memcpy(str, &g_shell_command[command_start], command_len);

            ret[size++] = str;

            break;
        }

        if (ch == ' ') {
            char *str = kmalloc(command_len + 1);
            memset(str, 0, command_len + 1);

            memcpy(str, &g_shell_command[command_start], command_len);

            ret[size++] = str;

            command_len = 0;
            command_start = off;

            while (g_shell_command[off] == ' ') {
                command_start++;

                off++;
            }
        }
    }

    if (arr_len != NULL) {
        *arr_len = size;
    }

    return ret;
}

void exec_command_shell() {
    if (!g_active_shell) return;

    int command_len = 0;
    char **commands = parse_shell_command(&command_len);

    if (!strcmp("1", commands[0])) {
        printk("run 1\n");
    } else if (!strcmp("print_super_block", commands[0])) {
        print_super_block();
    } else if (!strcmp("print_block_bitmap", commands[0])) {
        print_block_bitmap();
    } else if (!strcmp("reset_block_bitmap", commands[0])) {
        reset_block_bitmap();
    } else if (!strcmp("print_inode_bitmap", commands[0])) {
        print_inode_bitmap();
    } else if (!strcmp("reset_inode_bitmap", commands[0])) {
        reset_inode_bitmap();
    } else if (!strcmp("reset_bitmap", commands[0])) {
        reset_bitmap();
    } else if (!strcmp("print_bitmap", commands[0])) {
        print_bitmap();
    } else if (!strcmp("print_root_dir", commands[0])) {
        print_root_dir();
    } else if (!strcmp("ls", commands[0])) {
        ls_current_dir();
    } else if (!strcmp("mkdir", commands[0])) {
        for (int i = 1; i < command_len; ++i) {
            printk("===== start mkdir: %s =====\n", commands[i]);
            create_dir(commands[i]);
            printk("===== end mkdir: %s =====\n", commands[i]);
        }
    } else if (!strcmp("rm", commands[0])) {
        rm_directory(commands[1]);
    } else if (!strcmp("cd", commands[0])) {
        cd_directory(commands[1]);
    } else {
        for (int i = 0; i < command_len; ++i) {
            printk("%s ", commands[i]);
        }
        printk("\n");
    }

    for (int i = 0; i < command_len; ++i) {
        kfree_s(commands[i], strlen(commands[i]) + 1);
    }
    g_shell_command_off = 0;
    memset(g_shell_command, 0, 64);

    printk("[%s]> ", current->current_active_dir->name);
}

/**
 * 如果按了删除键，前面输入的内容要从g_shell_command中删掉
 */
void del_char_shell() {
    if (!g_active_shell || g_shell_command_off <= 0) return;
    g_shell_command[--g_shell_command_off] = 0;
}