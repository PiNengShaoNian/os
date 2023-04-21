#include "../include/stdio.h"
#include "../include/unistd.h"

void user_mode() {
    uactive_shell();

    while (true);
}
