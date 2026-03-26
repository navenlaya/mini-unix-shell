#ifndef PARSE_H
#define PARSE_H

/* Splits input into a NULL-terminated argv array. Caller must free the result. */
char **parse_input(char *input);

void free_argv(char **argv);

#endif
