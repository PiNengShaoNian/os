#ifndef OSKERNEL_TYPES_H
#define OSKERNEL_TYPES_H

#define EOF -1 // END OF FILE

#define NULL ((void *)0) // 空指针

#define EOS '\0' // 字符串结尾

#define bool _Bool
#define true 1
#define false 0

typedef unsigned int size_t;
typedef long long int64;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#endif