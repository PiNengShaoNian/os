#ifndef OSKERNEL_STDARG_H
#define OSKERNEL_STDARG_H

typedef char *va_list;
// count为可变参数的前一个参数,可变参数的地址从&count + sizeof(char*)开始
// 在32中sizeof(char*)为4
#define va_start(p, count) (p = (va_list)&count + sizeof(char*))
// 这行代码做了两件事情：1、修改p_args; 2、取值
#define va_arg(p, t) (*(t*)((p += sizeof(char*)) - sizeof(char*)))
#define va_end(p) (p = 0)
#endif