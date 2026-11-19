#ifndef APPOLAB_H
#define APPOLAB_H

#include <stdbool.h>
#include <stddef.h>

bool appolab_read_login(char *login, size_t capacity);
bool appolab_send_command(const char *command, char *response);

#endif
