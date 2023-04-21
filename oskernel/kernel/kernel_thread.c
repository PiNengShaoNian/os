#include "../include/linux/kernel.h"
#include "../include/linux/hd.h"
#include "../include/linux/fs.h"
#include "../include/linux/task.h"
#include "../include/linux/sys.h"
#include "../include/shell.h"
#include "../include/string.h"

extern task_t *current;
extern hd_t *g_active_hd;

task_t *wait_for_request = NULL;

void test_page_fault(const char *param) {
    if (!strcmp(param, "1")) {
        int cr2 = get_cr2();
        INFO_PRINT("cr2: 0x%x\n", cr2);

        // 虚拟内存只映射了4M，访问4M之外的就会发生缺页异常
        int *p = (int *) (4 * 1024 * 1024);
        *p = 1;

        INFO_PRINT("handle page fault success: %d\n", *p);
    } else if (!strcmp(param, "2")) {
        int cr2 = get_cr2();

        INFO_PRINT("cr2: 0x%x\n", cr2);

        handle_page_fault(cr2);
    }
}

void *kernel_thread_fun(void *arg) {
    hd_init();

    // 获取hdb盘的信息
    init_active_hd_info(g_active_hd->dev_no);
    init_active_hd_partition();

    // 为每个分区创建超级块
    init_super_block();

    // 重置位图
    reset_bitmap();

    // 创建根目录
    create_root_dir();

    // 打开根目录
    sys_open("/", O_RDWR);

    active_shell();

    while (true);
}