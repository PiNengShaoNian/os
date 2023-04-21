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

file_t *sys_open_file(const char *filename, const char *mode);

int sys_close_file(FILE *stream);

size_t sys_read_file(void *ptr, size_t size, FILE *stream);

#endif // OSKERNEL_SYS_H