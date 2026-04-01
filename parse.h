#ifndef PARSE_H
#define PARSE_H

#define MAX_ARGS 128
#define MAX_CMDS 16

typedef struct {
    char *argv[MAX_ARGS + 1];
    char *infile;
    char *outfile;
    int append;
} command_t;

typedef struct {
    command_t cmds[MAX_CMDS];
    int num_cmds;
    int background;
} pipeline_t;

/* Parse input line into a pipeline of commands. Returns 0 on success, -1 on empty input. */
int parse_input(char *input, pipeline_t *pl);

#endif
