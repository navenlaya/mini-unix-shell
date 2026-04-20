#ifndef BUILTINS_H
#define BUILTINS_H

#include "parse.h"
#include "history.h"

int is_builtin(const char *name);
int run_builtin(command_t *cmd, history_t *hist, int last_status);

#endif
