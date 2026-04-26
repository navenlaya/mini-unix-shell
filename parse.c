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

static int is_id_char(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
        || (c >= '0' && c <= '9') || c == '_';
}

static void append_str(char *out, int *oi, const char *s)
{
    while (*s && *oi < MAX_EXPAND - 1)
        out[(*oi)++] = *s++;
}

// Expand $VAR or ${VAR} starting at *pp (which points at '$').
// Advances *pp past the variable reference.
static void expand_var(const char **pp, char *out, int *oi)
{
    const char *p = *pp;
    p++; // skip $
    int braced = 0;
    if (*p == '{') {
        braced = 1;
        p++;
    }
    char name[256];
    int ni = 0;
    while (*p && is_id_char(*p) && ni < 255)
        name[ni++] = *p++;
    if (braced && *p == '}')
        p++;
    name[ni] = '\0';
    if (ni > 0) {
        const char *val = getenv(name);
        if (val)
            append_str(out, oi, val);
    } else if (*oi < MAX_EXPAND - 1) {
        out[(*oi)++] = '$';
    }
    *pp = p;
}

static void init_command(command_t *cmd)
{
    cmd->argv[0] = NULL;
    cmd->quoted[0] = 0;
    cmd->infile = NULL;
    cmd->outfile = NULL;
    cmd->append = 0;
}

// Read one operator or one word from *pp.
// Returns 1 on token, 0 on end-of-input, -1 on syntax error.
static int next_token(const char **pp, char *out, int *is_op, int *was_quoted)
{
    const char *p = *pp;
    *is_op = 0;
    *was_quoted = 0;

    while (*p == ' ' || *p == '\t')
        p++;
    if (*p == '\0') {
        *pp = p;
        return 0;
    }

    // Operators
    if (*p == '|' || *p == '<' || *p == '&') {
        out[0] = *p++;
        out[1] = '\0';
        *is_op = 1;
        *pp = p;
        return 1;
    }
    if (*p == '>') {
        if (*(p + 1) == '>') {
            out[0] = '>'; out[1] = '>'; out[2] = '\0';
            p += 2;
        } else {
            out[0] = '>'; out[1] = '\0';
            p++;
        }
        *is_op = 1;
        *pp = p;
        return 1;
    }

    // Word
    int oi = 0;
    int started = 0;
    enum { Q_NONE, Q_SINGLE, Q_DOUBLE } state = Q_NONE;

    // Tilde at start of unquoted word
    if (*p == '~') {
        char nx = *(p + 1);
        if (nx == '\0' || nx == '/' || nx == ' ' || nx == '\t'
                || nx == '|' || nx == '<' || nx == '>' || nx == '&') {
            const char *home = getenv("HOME");
            if (home) {
                append_str(out, &oi, home);
                p++;
                started = 1;
            }
        }
    }

    while (*p) {
        char c = *p;

        if (state == Q_NONE) {
            if (c == ' ' || c == '\t' || c == '|' || c == '<'
                    || c == '>' || c == '&')
                break;
            started = 1;
            if (c == '\'') {
                state = Q_SINGLE;
                *was_quoted = 1;
                p++;
            } else if (c == '"') {
                state = Q_DOUBLE;
                *was_quoted = 1;
                p++;
            } else if (c == '\\') {
                p++;
                if (*p) {
                    if (oi < MAX_EXPAND - 1)
                        out[oi++] = *p++;
                } else if (oi < MAX_EXPAND - 1) {
                    out[oi++] = '\\';
                }
            } else if (c == '$') {
                expand_var(&p, out, &oi);
            } else {
                if (oi < MAX_EXPAND - 1)
                    out[oi++] = c;
                p++;
            }
        } else if (state == Q_SINGLE) {
            // Single quotes: literal, no expansion
            if (c == '\'') {
                state = Q_NONE;
                p++;
            } else {
                if (oi < MAX_EXPAND - 1)
                    out[oi++] = c;
                p++;
            }
        } else {
            // Double quotes: $VAR expands; \ escapes only " \ $ `
            if (c == '"') {
                state = Q_NONE;
                p++;
            } else if (c == '\\') {
                char nx = *(p + 1);
                if (nx == '"' || nx == '\\' || nx == '$' || nx == '`') {
                    if (oi < MAX_EXPAND - 1)
                        out[oi++] = nx;
                    p += 2;
                } else {
                    if (oi < MAX_EXPAND - 1)
                        out[oi++] = '\\';
                    p++;
                }
            } else if (c == '$') {
                expand_var(&p, out, &oi);
            } else {
                if (oi < MAX_EXPAND - 1)
                    out[oi++] = c;
                p++;
            }
        }
    }

    if (state != Q_NONE) {
        fprintf(stderr, "mysh: syntax error: unmatched %s quote\n",
                state == Q_SINGLE ? "single" : "double");
        return -1;
    }

    if (!started) {
        *pp = p;
        return 0;
    }

    out[oi] = '\0';
    *pp = p;
    return 1;
}

int parse_input(char *input, pipeline_t *pl)
{
    pl->num_cmds = 0;
    pl->background = 0;
    pool_idx = 0;

    char *nl = strchr(input, '\n');
    if (nl)
        *nl = '\0';

    int ci = 0;
    init_command(&pl->cmds[ci]);
    int ai = 0;

    const char *p = input;
    char tokbuf[MAX_EXPAND];

    for (;;) {
        int is_op = 0, was_quoted = 0;
        int rc = next_token(&p, tokbuf, &is_op, &was_quoted);
        if (rc < 0)
            return -1;
        if (rc == 0)
            break;

        if (is_op) {
            if (strcmp(tokbuf, "|") == 0) {
                if (ai == 0) {
                    fprintf(stderr, "mysh: syntax error near '|'\n");
                    return -1;
                }
                pl->cmds[ci].argv[ai] = NULL;
                pl->cmds[ci].quoted[ai] = 0;
                ci++;
                if (ci >= MAX_CMDS) {
                    fprintf(stderr, "mysh: too many piped commands\n");
                    return -1;
                }
                init_command(&pl->cmds[ci]);
                ai = 0;
            } else if (strcmp(tokbuf, "<") == 0
                    || strcmp(tokbuf, ">") == 0
                    || strcmp(tokbuf, ">>") == 0) {
                char op[3];
                strcpy(op, tokbuf);
                int n_op = 0, n_q = 0;
                int nrc = next_token(&p, tokbuf, &n_op, &n_q);
                if (nrc <= 0 || n_op) {
                    fprintf(stderr, "mysh: syntax error near '%s'\n", op);
                    return -1;
                }
                char *slot = alloc_expand();
                strncpy(slot, tokbuf, MAX_EXPAND - 1);
                slot[MAX_EXPAND - 1] = '\0';
                if (op[0] == '<') {
                    pl->cmds[ci].infile = slot;
                } else {
                    pl->cmds[ci].outfile = slot;
                    pl->cmds[ci].append = (op[1] == '>');
                }
            } else if (strcmp(tokbuf, "&") == 0) {
                pl->background = 1;
            }
        } else {
            if (ai < MAX_ARGS) {
                char *slot = alloc_expand();
                strncpy(slot, tokbuf, MAX_EXPAND - 1);
                slot[MAX_EXPAND - 1] = '\0';
                pl->cmds[ci].argv[ai] = slot;
                pl->cmds[ci].quoted[ai] = was_quoted;
                ai++;
            }
        }
    }

    pl->cmds[ci].argv[ai] = NULL;
    pl->cmds[ci].quoted[ai] = 0;

    if (ai == 0 && ci == 0)
        return -1;
    if (ai == 0 && ci > 0) {
        fprintf(stderr, "mysh: syntax error near '|'\n");
        return -1;
    }

    pl->num_cmds = ci + 1;
    return 0;
}
