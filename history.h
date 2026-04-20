#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_MAX 100

typedef struct {
    char *entries[HISTORY_MAX];
    int count;
} history_t;

void history_init(history_t *h);
void history_add(history_t *h, const char *line);
void history_print(const history_t *h);
void history_free(history_t *h);

#endif
