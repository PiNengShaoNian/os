#ifndef OSKERNEL_SYS_H
#define OSKERNEL_SYS_H

#include "types.h"
#include "fs.h"

extern ssize_t sys_write(int fd, const void *buf, size_t count);

extern int sys_exit(int status);

extern int sys_fork();

extern pid_t sys_get_pid();

extern pid_t sys_get_ppid();

int sys_open(const char *pathname, int flags);

void sys_active_shell();

void sys_open_file(const char *filename, const char *mode);

#endif // OSKERNEL_SYS_H