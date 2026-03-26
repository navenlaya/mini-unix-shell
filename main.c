#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parse.h"
#include "execute.h"

#define INPUT_SIZE 1024

int main(void)
{
    char input[INPUT_SIZE];

    for (;;) {
        printf("mysh> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        char **argv = parse_input(input);
        if (!argv)
            continue;

        if (strcmp(argv[0], "exit") == 0) {
            free_argv(argv);
            break;
        }

        execute_command(argv);
        free_argv(argv);
    }

    return 0;
}
