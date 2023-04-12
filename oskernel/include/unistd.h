#ifndef OSKERNEL_UNISTD_H
#define OSKERNEL_UNISTD_H

#include "linux/types.h"

#define STDIN_FILENO     0
#define STDOUT_FILENO    1
#define STDERR_FILENO    2

#define __NR_write 0
#define __NR_exit    1
#define __NR_fork    2
#define __NR_get_pid    3
#define __NR_get_ppid   4


#define _syscall0(type, name) \
  type name(void) \
{ \
long __res; \
__asm__ volatile ("int 0x80" \
    : "=a" (__res) \
    : "0" (__NR_##name)); \
if (__res >= 0) \
    return (type) __res; \
errno = -__res; \
return -1; \
}

#define _syscall1(type, name, atype, a) \
type name(atype a) \
{ \
long __res; \
__asm__ volatile ("int 0x80" \
    : "=a" (__res) \
    : "0" (__NR_##name),"b" ((long)(a))); \
if (__res >= 0) \
    return (type) __res; \
errno = -__res; \
return -1; \
}

#define _syscall2(type, name, atype, a, btype, b) \
type name(atype a,btype b) \
{ \
long __res; \
__asm__ volatile ("int 0x80" \
    : "=a" (__res) \
    : "0" (__NR_##name),"b" ((long)(a)),"c" ((long)(b))); \
if (__res >= 0) \
    return (type) __res; \
errno = -__res; \
return -1; \
}

#define _syscall3(type, name, atype, a, btype, b, ctype, c) \
type name(atype a,btype b,ctype c) \
{ \
long __res; \
__asm__ volatile ("int 0x80" \
    : "=a" (__res) \
    : "0" (__NR_##name),"b" ((long)(a)),"c" ((long)(b)),"d" ((long)(c))); \
if (__res>=0) \
    return (type) __res; \
errno=-__res; \
return -1; \
}

extern int errno;

int write(int fildes, const char *buf, int count);

pid_t fork();

pid_t getpid();

pid_t getppid();


#endif // OSKERNEL_UNISTD_H