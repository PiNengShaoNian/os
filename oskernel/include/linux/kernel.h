#ifndef OSKERNEL_KERNEL_H
#define OSKERNEL_KERNEL_H

#include "../stdarg.h"
#include "types.h"

int vsprintf(char *buf, const char *fmt, va_list args);

int printk(const char *fmt, ...);

uint get_cr3();

void set_cr3(uint v);

void enable_page();

uint get_cr2();

/*=================================
 *  自定义打印输出
 ==================================*/
#define INFO_OUTPUT         3
#define WARNING_OUTPUT      2
#define DEBUG_OUTPUT        1
#define ERROR_OUTPUT        0

#define DEBUG
#define DEBUG_LEVEL         INFO_OUTPUT

#define KPRINT(info, ...) do{ \
    printk("[Info] (%s:%d):" info"", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}while(0)

#define INFO_PRINT(info, ...) do{ \
    if(DEBUG_LEVEL>=INFO_OUTPUT){ \
        printk("[Info] (%s:%d):" info"", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
}while(0)

#define WARNING_PRINT(info, ...) do{ \
    if(DEBUG_LEVEL>=WARNING_OUTPUT){ \
        printk("[Info] (%s:%d):" info"", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
}while(0)

#define DEBUG_PRINT(info, ...) do{ \
    if(DEBUG_LEVEL>=DEBUG_OUTPUT){ \
        printk("[Info] (%s:%d):" info"", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
}while(0)

#define ERROR_PRINT(info, ...) do{ \
    if(DEBUG_LEVEL>=ERROR_OUTPUT){ \
        printk("[Info] (%s:%d):" info"", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
}while(0)

#endif