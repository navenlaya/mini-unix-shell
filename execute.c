#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <glob.h>
#include <sys/wait.h>
#include "execute.h"

static void restore_signals(void)
{
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
}

static int setup_redirections(command_t *cmd)
{
    if (cmd->infile) {
        int fd = open(cmd->infile, O_RDONLY);
        if (fd < 0) {
            perror(cmd->infile);
            return -1;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (cmd->outfile) {
        int flags = O_WRONLY | O_CREAT | (cmd->append ? O_APPEND : O_TRUNC);
        int fd = open(cmd->outfile, flags, 0644);
        if (fd < 0) {
            perror(cmd->outfile);
            return -1;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    return 0;
}

static int has_glob_chars(const char *s)
{
    for (; *s; s++) {
        if (*s == '*' || *s == '?' || *s == '[')
            return 1;
    }
    return 0;
}

static void expand_globs(command_t *cmd)
{
    static char *expanded[MAX_ARGS + 1];
    int ei = 0;

    for (int i = 0; cmd->argv[i] && ei < MAX_ARGS; i++) {
        if (!has_glob_chars(cmd->argv[i])) {
            expanded[ei++] = cmd->argv[i];
            continue;
        }
        glob_t g;
        int ret = glob(cmd->argv[i], GLOB_NOCHECK, NULL, &g);
        if (ret != 0) {
            expanded[ei++] = cmd->argv[i];
            continue;
        }
        for (size_t j = 0; j < g.gl_pathc && ei < MAX_ARGS; j++)
            expanded[ei++] = g.gl_pathv[j];
        /* intentionally not calling globfree — child will exec or exit soon */
    }
    expanded[ei] = NULL;

    for (int i = 0; i <= ei; i++)
        cmd->argv[i] = expanded[i];
}

static void run_child(command_t *cmd)
{
    restore_signals();
    if (setup_redirections(cmd) < 0)
        exit(EXIT_FAILURE);
    expand_globs(cmd);
    execvp(cmd->argv[0], cmd->argv);
    fprintf(stderr, "%s: command not found\n", cmd->argv[0]);
    exit(127);
}

static void cleanup_children(pid_t *pids, int count)
{
    for (int i = 0; i < count; i++) {
        kill(pids[i], SIGTERM);
        waitpid(pids[i], NULL, 0);
    }
}

int execute_pipeline(pipeline_t *pl)
{
    int n = pl->num_cmds;
    int prev_fd = -1;
    pid_t pids[MAX_CMDS];
    int spawned = 0;

    for (int i = 0; i < n; i++) {
        int pipefd[2] = {-1, -1};

        if (i < n - 1) {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                cleanup_children(pids, spawned);
                if (prev_fd != -1)
                    close(prev_fd);
                return -1;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            if (pipefd[0] != -1) {
                close(pipefd[0]);
                close(pipefd[1]);
            }
            cleanup_children(pids, spawned);
            if (prev_fd != -1)
                close(prev_fd);
            return -1;
        }

        if (pid == 0) {
            if (prev_fd != -1) {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }
            if (pipefd[1] != -1) {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }
            run_child(&pl->cmds[i]);
        }

        pids[i] = pid;
        spawned++;

        if (prev_fd != -1)
            close(prev_fd);
        if (pipefd[1] != -1)
            close(pipefd[1]);

        prev_fd = pipefd[0];
    }

    if (!pl->background) {
        int status = 0;
        for (int i = 0; i < n; i++)
            waitpid(pids[i], &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }

    printf("[bg] %d\n", pids[n - 1]);
    return 0;
}
