#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "builtins.h"

static int builtin_cd(char **argv)
{
    const char *dir = argv[1];
    if (!dir) {
        dir = getenv("HOME");
        if (!dir) {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
    }
    if (argv[1] && argv[2]) {
        fprintf(stderr, "cd: too many arguments\n");
        return 1;
    }
    if (chdir(dir) < 0) {
        perror("cd");
        return 1;
    }
    return 0;
}

static int builtin_status(int last_status)
{
    printf("%d\n", last_status);
    return 0;
}

int is_builtin(const char *name)
{
    return strcmp(name, "cd") == 0
        || strcmp(name, "exit") == 0
        || strcmp(name, "history") == 0
        || strcmp(name, "status") == 0;
}

int run_builtin(command_t *cmd, history_t *hist, int last_status)
{
    if (strcmp(cmd->argv[0], "cd") == 0)
        return builtin_cd(cmd->argv);
    if (strcmp(cmd->argv[0], "history") == 0) {
        history_print(hist);
        return 0;
    }
    if (strcmp(cmd->argv[0], "status") == 0)
        return builtin_status(last_status);
    return 1;
}
