#ifndef OSKERNEL_TRAPS_H
#define OSKERNEL_TRAPS_H

#include "head.h"

void gdt_init();
void idt_init();

#endif // OSKERNEL_TRAPS_H