#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "execute.h"

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

static void run_child(command_t *cmd)
{
    if (setup_redirections(cmd) < 0)
        exit(EXIT_FAILURE);
    execvp(cmd->argv[0], cmd->argv);
    fprintf(stderr, "%s: command not found\n", cmd->argv[0]);
    exit(EXIT_FAILURE);
}

int execute_pipeline(pipeline_t *pl)
{
    int n = pl->num_cmds;
    int prev_fd = -1;
    pid_t pids[MAX_CMDS];

    for (int i = 0; i < n; i++) {
        int pipefd[2] = {-1, -1};

        if (i < n - 1) {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                return -1;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
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
        return status;
    }

    printf("[bg] %d\n", pids[n - 1]);
    return 0;
}
