#ifndef OSKERNEL_SYS_H
#define OSKERNEL_SYS_H

#include "types.h"

extern ssize_t sys_write(int fd, const void *buf, size_t count);

extern int sys_exit(int status);

#endif // OSKERNEL_SYS_H