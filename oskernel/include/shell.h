#ifndef OS_SHELL_H
#define OS_SHELL_H

bool shell_is_active();

bool active_shell();

bool close_shell();

void exec_command_shell();

void put_char_shell(char ch);

void del_char_shell();

typedef struct _filepath_parse_result {
    u32 depth;
    char **data;
} filepath_parse_result;

#endif // OS_SHELL_H