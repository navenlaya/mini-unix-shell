#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include "parse.h"
#include "execute.h"

#define INPUT_SIZE 1024

static void sigchld_handler(int sig)
{
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

static void setup_signals(void)
{
    struct sigaction sa;

    /* ignore SIGINT and SIGTSTP in the shell process */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);

    /* reap background children automatically */
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
}

int main(void)
{
    char input[INPUT_SIZE];
    pipeline_t pl;

    setup_signals();

    for (;;) {
        printf("mysh> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        if (parse_input(input, &pl) < 0)
            continue;

        if (strcmp(pl.cmds[0].argv[0], "exit") == 0)
            break;

        execute_pipeline(&pl);
    }

    return 0;
}
