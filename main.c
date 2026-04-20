#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include "parse.h"
#include "execute.h"
#include "builtins.h"
#include "history.h"

#define INPUT_SIZE 1024

static volatile sig_atomic_t got_sigint;

static void sigint_handler(int sig)
{
    (void)sig;
    got_sigint = 1;
}

static void sigchld_handler(int sig)
{
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

static void setup_signals(void)
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigaction(SIGTSTP, &sa, NULL);

    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
}

static void drain_overlong(void)
{
    int c;
    while ((c = fgetc(stdin)) != EOF && c != '\n')
        ;
}

int main(void)
{
    char input[INPUT_SIZE];
    pipeline_t pl;
    history_t hist;
    int last_status = 0;

    history_init(&hist);
    setup_signals();

    for (;;) {
        got_sigint = 0;
        printf("mysh> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            if (got_sigint) {
                clearerr(stdin);
                printf("\n");
                continue;
            }
            break;
        }

        if (got_sigint) {
            printf("\n");
            continue;
        }

        if (!strchr(input, '\n') && !feof(stdin)) {
            fprintf(stderr, "mysh: input too long\n");
            drain_overlong();
            continue;
        }

        /* save raw line before parse mutates it */
        char save[INPUT_SIZE];
        strncpy(save, input, sizeof(save));
        save[sizeof(save) - 1] = '\0';
        char *nl = strchr(save, '\n');
        if (nl)
            *nl = '\0';

        if (parse_input(input, &pl) < 0)
            continue;

        if (strcmp(pl.cmds[0].argv[0], "exit") == 0)
            break;

        history_add(&hist, save);

        if (pl.num_cmds == 1 && is_builtin(pl.cmds[0].argv[0])) {
            last_status = run_builtin(&pl.cmds[0], &hist, last_status);
            continue;
        }

        last_status = execute_pipeline(&pl);
        if (last_status < 0)
            last_status = 1;
    }

    history_free(&hist);
    return last_status;
}
