#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

#define EXPAND_POOL_SIZE (MAX_ARGS + 4)
static char expand_pool[EXPAND_POOL_SIZE][MAX_EXPAND];
static int pool_idx;

static char *alloc_expand(void)
{
    if (pool_idx >= EXPAND_POOL_SIZE)
        pool_idx = 0;
    return expand_pool[pool_idx++];
}

static char *expand_tilde(char *tok)
{
    if (tok[0] != '~')
        return tok;
    const char *home = getenv("HOME");
    if (!home)
        return tok;
    if (tok[1] == '\0' || tok[1] == '/') {
        char *buf = alloc_expand();
        snprintf(buf, MAX_EXPAND, "%s%s", home, tok + 1);
        return buf;
    }
    return tok;
}

static char *expand_vars(char *tok)
{
    if (!strchr(tok, '$'))
        return tok;

    char *buf = alloc_expand();
    char *out = buf;
    char *end = buf + MAX_EXPAND - 1;
    const char *p = tok;

    while (*p && out < end) {
        if (*p == '$') {
            p++;
            char name[256];
            int ni = 0;
            int braced = 0;
            if (*p == '{') {
                braced = 1;
                p++;
            }
            while (*p && ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z')
                    || (*p >= '0' && *p <= '9') || *p == '_') && ni < 255) {
                name[ni++] = *p++;
            }
            if (braced && *p == '}')
                p++;
            name[ni] = '\0';
            if (ni > 0) {
                const char *val = getenv(name);
                if (val) {
                    while (*val && out < end)
                        *out++ = *val++;
                }
            } else {
                *out++ = '$';
            }
        } else {
            *out++ = *p++;
        }
    }
    *out = '\0';
    return buf;
}

static char *expand_token(char *tok)
{
    tok = expand_tilde(tok);
    tok = expand_vars(tok);
    return tok;
}

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
    pool_idx = 0;

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
            pl->cmds[ci].infile = expand_token(tok);
        } else if (strcmp(tok, ">>") == 0) {
            tok = strtok(NULL, " \t");
            if (!tok) {
                fprintf(stderr, "mysh: syntax error near '>>'\n");
                return -1;
            }
            pl->cmds[ci].outfile = expand_token(tok);
            pl->cmds[ci].append = 1;
        } else if (strcmp(tok, ">") == 0) {
            tok = strtok(NULL, " \t");
            if (!tok) {
                fprintf(stderr, "mysh: syntax error near '>'\n");
                return -1;
            }
            pl->cmds[ci].outfile = expand_token(tok);
        } else if (strcmp(tok, "&") == 0) {
            pl->background = 1;
        } else {
            if (ai < MAX_ARGS)
                pl->cmds[ci].argv[ai++] = expand_token(tok);
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
