#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "history.h"

void history_init(history_t *h)
{
    h->count = 0;
    for (int i = 0; i < HISTORY_MAX; i++)
        h->entries[i] = NULL;
}

void history_add(history_t *h, const char *line)
{
    if (h->count < HISTORY_MAX) {
        h->entries[h->count++] = strdup(line);
    } else {
        free(h->entries[0]);
        for (int i = 1; i < HISTORY_MAX; i++)
            h->entries[i - 1] = h->entries[i];
        h->entries[HISTORY_MAX - 1] = strdup(line);
    }
}

void history_print(const history_t *h)
{
    for (int i = 0; i < h->count; i++)
        printf("  %d  %s\n", i + 1, h->entries[i]);
}

void history_free(history_t *h)
{
    for (int i = 0; i < h->count; i++)
        free(h->entries[i]);
    h->count = 0;
}
