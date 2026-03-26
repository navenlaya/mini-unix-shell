#include <stdlib.h>
#include <string.h>
#include "parse.h"

#define MAX_TOKENS 128

char **parse_input(char *input)
{
    char **argv = malloc(sizeof(char *) * (MAX_TOKENS + 1));
    if (!argv)
        return NULL;

    int i = 0;
    char *token = strtok(input, " \t\n");
    while (token && i < MAX_TOKENS) {
        argv[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    argv[i] = NULL;

    if (i == 0) {
        free(argv);
        return NULL;
    }

    return argv;
}

void free_argv(char **argv)
{
    free(argv);
}
