#include "../include/asm/system.h"
#include "../include/linux/kernel.h"
#include "../include/linux/mm.h"
#include "../include/linux/task.h"
#include "../include/string.h"

// 需要完整映射4g大小的物理内存，需要1个大4096字节的PDT表
// 和1024个4096字节大小的PT, 所以总共所需 4096 + 1024 * 4096

#define PDT_START_ADDR 0x20000

// 线性地址从2M开始用
#define VIRTUAL_MEM_START 0x200000

extern task_t *current;

int *g_pdt = (int *) 0x20000;

void *virtual_memory_init() {
    int *pdt = (int *) PDT_START_ADDR;

    // 清零
    memset(pdt, 0, PAGE_SIZE);

#define NPTE 1024

    // PDE和PTE的低三位控制位含义
    // P（Present）：用于标识该页表是否存在，若为1则存在，若为0则不存在。
    // R/W（Read/Write）：用于标识该页表的读写权限，若为1则可读写，若为0则只读。
    // U/S（User/Supervisor）：用于标识该页表的权限级别，若为1则表示该页表可以被用户程序访问，若为0则只能被系统级程序访问。
    for (int i = 0; i < 4; ++i) {
        // pdt里面的每项，即pde，内容是ptt + 尾12位的权限位
        int page_table = (int) PDT_START_ADDR + ((i + 1) * 4096);
        int pde = 0b00000000000000000000000000000111 | page_table;

        pdt[i] = pde;

        int *pte_arr = (int *) page_table;

        if (i == 0) {
            // 第一块映射区，给内核用
            for (int j = 0; j < 0x400; ++j) {
                int *pte_p = &pte_arr[j];

                int virtual_addr = j * PAGE_SIZE;
                *pte_p = 0b00000000000000000000000000000111 | virtual_addr;
            }
        } else {
//            for (int j = 0; j < NPTE; ++j) {
//                int *pte_p = &pte_arr[j];
//
//                int virtual_addr = j * PAGE_SIZE;
//                virtual_addr = virtual_addr + i * NPTE * PAGE_SIZE;
//
//                *pte_p = 0b00000000000000000000000000000111 | virtual_addr;
//            }
        }
    }

    BOCHS_DEBUG_MAGIC

    set_cr3((uint) pdt);

    enable_page();

    BOCHS_DEBUG_MAGIC

    printk("pdt addr: 0x%p\n", pdt);

    return pdt;
}

int get_pde_by_addr(int addr) {
    int index = addr >> 22;

    INFO_PRINT("pdt index: %d\n", index);

    int res = *(int *) (g_pdt + index);

    INFO_PRINT("pde val: 0x%x\n", res);

    return res;
}

int get_pte_by_addr(int addr) {
    int pde_index = addr >> 22;
    int pte_index = addr >> 12 & 0x3ff;

    INFO_PRINT("pde_index: %d, pte_index: %d\n", pde_index, pte_index);

    int *page_table_addr = *(int **) (g_pdt + pde_index);
    INFO_PRINT("page_table_addr: 0x%08x\n", page_table_addr);

    page_table_addr = (int *) ((int) page_table_addr & 0xfffff000);


    int pte = *(page_table_addr + pte_index);
    INFO_PRINT("pte val: 0x%08x\n", pte);

    return pte;
}

static int *get_pte_addr_by_addr(int addr) {
    int pde_index = addr >> 22;
    int pte_index = addr >> 12 & 0x3ff;

    INFO_PRINT("pde_index: %d, pte_index: %d\n", pde_index, pte_index);

    int *page_table_addr = *(int **) (g_pdt + pde_index);

    int *pte_addr = page_table_addr + pte_index;

    return (int *) ((int) pte_addr & 0xfffff000);
}

void handle_page_fault(int addr) {
    INFO_PRINT("===== start handle page fault ====\n");

    int pte = get_pte_by_addr(addr);

    if (pte == 0) {
        ERROR_PRINT("pte == 0, start handle..\n");

        void *page = get_free_page();
        int *pte_addr = get_pte_addr_by_addr(addr);

        INFO_PRINT("phyics page: 0x%08x\n", page);

        *pte_addr = (int) page | 0x007;
    }
}