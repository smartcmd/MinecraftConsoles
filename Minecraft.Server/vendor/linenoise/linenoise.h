#ifndef VENDORED_LINENOISE_H
#define VENDORED_LINENOISE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct linenoiseCompletions {
    size_t len;
    char **cvec;
} linenoiseCompletions;

typedef void(linenoiseCompletionCallback)(const char *buf, linenoiseCompletions *lc);

char *linenoise(const char *prompt);
void linenoiseFree(void *ptr);

void linenoiseSetCompletionCallback(linenoiseCompletionCallback *fn);
void linenoiseAddCompletion(linenoiseCompletions *lc, const char *str);

int linenoiseHistoryAdd(const char *line);
int linenoiseHistorySetMaxLen(int len);

void linenoiseRequestStop(void);
void linenoiseResetStop(void);

/* Wrap external stdout/stderr writes so active prompt can be cleared/restored safely. */
void linenoiseExternalWriteBegin(void);
void linenoiseExternalWriteEnd(void);

#ifdef __cplusplus
}
#endif

#endif
