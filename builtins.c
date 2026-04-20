#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "builtins.h"

extern char **environ;

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

static int builtin_export(char **argv)
{
    if (!argv[1]) {
        for (char **env = environ; *env; env++)
            printf("%s\n", *env);
        return 0;
    }
    for (int i = 1; argv[i]; i++) {
        char *eq = strchr(argv[i], '=');
        if (!eq) {
            fprintf(stderr, "export: usage: export VAR=value\n");
            return 1;
        }
        *eq = '\0';
        if (setenv(argv[i], eq + 1, 1) != 0) {
            perror("export");
            *eq = '=';
            return 1;
        }
        *eq = '=';
    }
    return 0;
}

static int builtin_unset(char **argv)
{
    if (!argv[1]) {
        fprintf(stderr, "unset: usage: unset VAR\n");
        return 1;
    }
    for (int i = 1; argv[i]; i++)
        unsetenv(argv[i]);
    return 0;
}

static int builtin_status(int last_status)
{
    printf("%d\n", last_status);
    return 0;
}

static int builtin_help(void)
{
    printf("mysh - mini unix shell\n\n");
    printf("Built-in commands:\n");
    printf("  cd [dir]              Change directory (default: $HOME)\n");
    printf("  exit                  Exit the shell\n");
    printf("  history               Show command history\n");
    printf("  status                Print last exit code\n");
    printf("  export VAR=value      Set environment variable\n");
    printf("  unset VAR             Remove environment variable\n");
    printf("  help                  Show this message\n\n");
    printf("Operators:\n");
    printf("  cmd1 | cmd2           Pipe output of cmd1 to cmd2\n");
    printf("  cmd < file            Redirect stdin from file\n");
    printf("  cmd > file            Redirect stdout to file\n");
    printf("  cmd >> file           Append stdout to file\n");
    printf("  cmd &                 Run command in background\n\n");
    printf("Expansions:\n");
    printf("  ~                     Expands to $HOME\n");
    printf("  $VAR                  Expands to environment variable\n");
    printf("  *.txt                 Glob/wildcard expansion\n");
    return 0;
}

int is_builtin(const char *name)
{
    return strcmp(name, "cd") == 0
        || strcmp(name, "exit") == 0
        || strcmp(name, "history") == 0
        || strcmp(name, "status") == 0
        || strcmp(name, "export") == 0
        || strcmp(name, "unset") == 0
        || strcmp(name, "help") == 0;
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
    if (strcmp(cmd->argv[0], "export") == 0)
        return builtin_export(cmd->argv);
    if (strcmp(cmd->argv[0], "unset") == 0)
        return builtin_unset(cmd->argv);
    if (strcmp(cmd->argv[0], "help") == 0)
        return builtin_help();
    return 1;
}
