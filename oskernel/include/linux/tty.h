#ifndef OSKERNEL_TTY_H
#define OSKERNEL_TTY_H

#include "types.h"

void console_init(void);
int console_write(char *buf, u32 count);

#endif