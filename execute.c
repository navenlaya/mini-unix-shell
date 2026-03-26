#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "execute.h"

int execute_command(char **argv)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        execvp(argv[0], argv);
        fprintf(stderr, "Command execution failed\n");
        exit(EXIT_FAILURE);
    }

    int status;
    waitpid(pid, &status, 0);
    return status;
}
