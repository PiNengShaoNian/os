#ifndef OSKERNEL_SYS_H
#define OSKERNEL_SYS_H

#include "types.h"

extern ssize_t sys_write(int fd, const void *buf, size_t count);

#endif // OSKERNEL_SYS_H