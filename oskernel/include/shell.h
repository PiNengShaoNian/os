#ifndef OS_SHELL_H
#define OS_SHELL_H

bool shell_is_active();

bool active_shell();

bool close_shell();

void exec_command_shell();

void put_char_shell(char ch);

void del_char_shell();

#endif // OS_SHELL_H