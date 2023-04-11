#include "../include/stdio.h"
#include "../include/stdarg.h"
#include "../include/unistd.h"

extern int vsprintf(char *buf, const char *fmt, va_list args);

static char buf[1024];

int printf(const char *fmt, ...) {
    va_list args;
    int i;

    va_start(args, fmt);

    i = vsprintf(buf, fmt, args);

    va_end(args);

    write(STDOUT_FILENO, buf, i);

    return i;
}