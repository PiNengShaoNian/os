#include "../include/linux/kernel.h"

// 在x86架构中，CR3寄存器是控制寄存器(Control Register)之一，用于存储页目录表(Page Directory Table)的基地址。
// 页目录表是一种数据结构，用于将虚拟地址映射到物理地址，从而实现虚拟内存的机制。
// 当CPU需要访问一个虚拟地址时，它会首先通过CR3寄存器中存储的页目录表的基地址找到页目录表的入口，
// 然后根据虚拟地址的高10位找到对应的页目录项(Page Directory Entry)，并从中获取该虚拟地址对应的页表(Page Table)的基地址。
// 最后，根据虚拟地址的中间10位找到对应的页表项(Page Table Entry)，并从中获取该虚拟地址在物理内存中对应的物理地址。
// 因此，CR3寄存器在x86中扮演着关键的角色，它直接影响了虚拟地址到物理地址的映射。当操作系统需要切换进程时，
// 需要更新CR3寄存器中存储的页目录表的基地址，以切换到新的地址空间。

inline uint

get_cr3() {
    asm volatile("mov eax, cr3;");
}

inline void set_cr3(uint v) {
    asm volatile("mov cr3, eax;"::"a"(v));
}

// 将cr0的第31位PG置为1以开启分页模式
inline void enable_page() {
    asm volatile("mov eax, cr0;"
                 "or eax, 0x80000000;"
                 "mov cr0, eax;"
            );
}

uint get_cr2() {
    asm volatile("mov eax, cr2;");
}