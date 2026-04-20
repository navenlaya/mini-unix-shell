#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

static void init_command(command_t *cmd)
{
    cmd->argv[0] = NULL;
    cmd->infile = NULL;
    cmd->outfile = NULL;
    cmd->append = 0;
}

int parse_input(char *input, pipeline_t *pl)
{
    pl->num_cmds = 0;
    pl->background = 0;

    char *nl = strchr(input, '\n');
    if (nl)
        *nl = '\0';

    while (*input == ' ' || *input == '\t')
        input++;
    if (*input == '\0')
        return -1;

    int ci = 0;
    init_command(&pl->cmds[ci]);
    int ai = 0;

    char *tok = strtok(input, " \t");
    while (tok) {
        if (strcmp(tok, "|") == 0) {
            if (ai == 0) {
                fprintf(stderr, "mysh: syntax error near '|'\n");
                return -1;
            }
            pl->cmds[ci].argv[ai] = NULL;
            ci++;
            if (ci >= MAX_CMDS) {
                fprintf(stderr, "mysh: too many piped commands\n");
                return -1;
            }
            init_command(&pl->cmds[ci]);
            ai = 0;
        } else if (strcmp(tok, "<") == 0) {
            tok = strtok(NULL, " \t");
            if (!tok) {
                fprintf(stderr, "mysh: syntax error near '<'\n");
                return -1;
            }
            pl->cmds[ci].infile = tok;
        } else if (strcmp(tok, ">>") == 0) {
            tok = strtok(NULL, " \t");
            if (!tok) {
                fprintf(stderr, "mysh: syntax error near '>>'\n");
                return -1;
            }
            pl->cmds[ci].outfile = tok;
            pl->cmds[ci].append = 1;
        } else if (strcmp(tok, ">") == 0) {
            tok = strtok(NULL, " \t");
            if (!tok) {
                fprintf(stderr, "mysh: syntax error near '>'\n");
                return -1;
            }
            pl->cmds[ci].outfile = tok;
        } else if (strcmp(tok, "&") == 0) {
            pl->background = 1;
        } else {
            if (ai < MAX_ARGS)
                pl->cmds[ci].argv[ai++] = tok;
        }
        tok = strtok(NULL, " \t");
    }

    pl->cmds[ci].argv[ai] = NULL;

    if (ai == 0 && ci == 0)
        return -1;
    if (ai == 0 && ci > 0) {
        fprintf(stderr, "mysh: syntax error near '|'\n");
        return -1;
    }

    pl->num_cmds = ci + 1;
    return 0;
}
