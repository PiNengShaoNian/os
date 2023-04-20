#include "../include/stdio.h"
#include "../include/unistd.h"

void user_mode() {
    char *str = "hello world!";
    printf("%s, %d\n", str, 11);

    pid_t pid = fork();
    if (pid > 0) {
        printf("pid=%d, ppid=%d\n", getpid(), getppid());
    } else if (pid == 0) {
        printf("pid=%d, ppid=%d\n", getpid(), getppid());

        for (int i = 0; i < 10; ++i) {
            printf("%d\n", i);
        }
    }
}
