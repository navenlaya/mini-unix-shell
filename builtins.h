#ifndef BUILTINS_H
#define BUILTINS_H

#include "parse.h"

/* Returns 1 if argv[0] is a builtin, 0 otherwise. */
int is_builtin(const char *name);

/* Runs the builtin. Returns 0 on success, -1 on error. */
int run_builtin(command_t *cmd);

#endif
