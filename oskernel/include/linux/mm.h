#ifndef OSKERNEL_MM_H
#define OSKERNEL_MM_H

#include "types.h"

#define PAGE_SIZE 4096

typedef struct {
    unsigned int base_addr_low; // 内存基地址的低32位
    unsigned int base_addr_high; // 内存基地址的高32位
    unsigned int length_low; // 内存块长度的低32位
    unsigned int length_high; // 内存块长度的高32位
    unsigned int type; // 描述内存块的类型
} check_memory_item_t;

typedef struct {
    unsigned short times;
    check_memory_item_t *data;
} check_memory_info_t;

typedef struct {
    uint addr_start;     // 可用内存起始地址 一般是1M
    uint addr_end;       // 可用内存结束地址
    uint valid_mem_size;
    uint pages_total;    // 机器物理内存共多少page
    uint pages_free;     // 机器物理内存还剩多少page
    uint pages_used;     // 机器物理内存用了多少page
} physics_memory_info_t;

typedef struct {
    uint addr_base;          // 可用物理内存开始位置  3M
    uint pages_total;        // 共有多少page   机器物理内存共多少page - 0x30000（3M）
    uint bitmap_item_used;  // 如果1B映射一个page，用了多少个page
    uchar *map;
} physics_memory_map_t;

void print_check_memory_info();

void memory_init();

void memory_map_init();

void *virtual_memory_init();

void *get_free_page();

void free_page(void *p);

// 分配、释放虚拟内存
void *kmalloc(size_t size);

#endif // OSKERNEL_MM_H