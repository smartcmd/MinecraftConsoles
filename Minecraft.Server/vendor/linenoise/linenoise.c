#include "linenoise.h"

#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define LINENOISE_MAX_LINE 4096
#define LINENOISE_MAX_PROMPT 128

typedef struct linenoiseHistory {
    char **items;
    int len;
    int cap;
    int maxLen;
} linenoiseHistory;

static linenoiseCompletionCallback *g_completionCallback = NULL;
static volatile LONG g_stopRequested = 0;
static linenoiseHistory g_history = { NULL, 0, 0, 128 };
/* Guards redraw/log interleaving so prompt and log lines do not overlap. */
static CRITICAL_SECTION g_ioLock;
static volatile LONG g_ioLockState = 0; /* 0=not init, 1=init in progress, 2=ready */
/* Snapshot of current editor line used to restore prompt after external output. */
static volatile LONG g_editorActive = 0;
static char g_editorPrompt[LINENOISE_MAX_PROMPT] = { 0 };
static char g_editorBuf[LINENOISE_MAX_LINE] = { 0 };
static int g_editorLen = 0;
static int g_editorPos = 0;
static int g_editorPrevLen = 0;

/**
 * Lazily initialize the console I/O critical section.
 * This avoids static init order issues and keeps startup cost minimal.
 */
static void linenoiseEnsureIoLockInit(void)
{
    LONG state = InterlockedCompareExchange(&g_ioLockState, 0, 0);
    if (state == 2)
        return;

    if (state == 0 && InterlockedCompareExchange(&g_ioLockState, 1, 0) == 0)
    {
        InitializeCriticalSection(&g_ioLock);
        InterlockedExchange(&g_ioLockState, 2);
        return;
    }

    while (InterlockedCompareExchange(&g_ioLockState, 0, 0) != 2)
    {
        Sleep(0);
    }
}

static void linenoiseLockIo(void)
{
    linenoiseEnsureIoLockInit();
    EnterCriticalSection(&g_ioLock);
}

static void linenoiseUnlockIo(void)
{
    LeaveCriticalSection(&g_ioLock);
}

/**
 * Save current prompt/buffer/cursor state for later redraw.
 * Called after each redraw while editor is active.
 */
static void linenoiseUpdateEditorState(const char *prompt, const char *buf, int len, int pos, int prevLen)
{
    if (prompt == NULL)
        prompt = "";
    if (buf == NULL)
        buf = "";

    strncpy_s(g_editorPrompt, sizeof(g_editorPrompt), prompt, _TRUNCATE);
    strncpy_s(g_editorBuf, sizeof(g_editorBuf), buf, _TRUNCATE);
    g_editorLen = len;
    g_editorPos = pos;
    g_editorPrevLen = prevLen;
    InterlockedExchange(&g_editorActive, 1);
}

static void linenoiseDeactivateEditorState(void)
{
    InterlockedExchange(&g_editorActive, 0);
    g_editorPrompt[0] = 0;
    g_editorBuf[0] = 0;
    g_editorLen = 0;
    g_editorPos = 0;
    g_editorPrevLen = 0;
}

static char *linenoiseStrdup(const char *src)
{
    size_t n = strlen(src) + 1;
    char *out = (char *)malloc(n);
    if (out == NULL)
        return NULL;
    memcpy(out, src, n);
    return out;
}

static void linenoiseEnsureHistoryCapacity(int wanted)
{
    if (wanted <= g_history.cap)
        return;

    int newCap = g_history.cap == 0 ? 32 : g_history.cap;
    while (newCap < wanted)
        newCap *= 2;

    char **newItems = (char **)realloc(g_history.items, sizeof(char *) * (size_t)newCap);
    if (newItems == NULL)
        return;

    g_history.items = newItems;
    g_history.cap = newCap;
}

static void linenoiseClearCompletions(linenoiseCompletions *lc)
{
    size_t i = 0;
    for (i = 0; i < lc->len; ++i)
    {
        free(lc->cvec[i]);
    }
    free(lc->cvec);
    lc->cvec = NULL;
    lc->len = 0;
}

void linenoiseAddCompletion(linenoiseCompletions *lc, const char *str)
{
    char **newVec = (char **)realloc(lc->cvec, sizeof(char *) * (lc->len + 1));
    if (newVec == NULL)
        return;

    lc->cvec = newVec;
    lc->cvec[lc->len] = linenoiseStrdup(str);
    if (lc->cvec[lc->len] == NULL)
        return;

    lc->len += 1;
}

void linenoiseSetCompletionCallback(linenoiseCompletionCallback *fn)
{
    g_completionCallback = fn;
}

void linenoiseFree(void *ptr)
{
    free(ptr);
}

int linenoiseHistorySetMaxLen(int len)
{
    if (len <= 0)
        return 0;

    g_history.maxLen = len;
    while (g_history.len > g_history.maxLen)
    {
        free(g_history.items[0]);
        memmove(g_history.items, g_history.items + 1, sizeof(char *) * (size_t)(g_history.len - 1));
        g_history.len -= 1;
    }

    return 1;
}

int linenoiseHistoryAdd(const char *line)
{
    if (line == NULL || line[0] == 0)
        return 0;

    if (g_history.len > 0)
    {
        const char *last = g_history.items[g_history.len - 1];
        if (last != NULL && strcmp(last, line) == 0)
            return 1;
        }

    linenoiseEnsureHistoryCapacity(g_history.len + 1);
    if (g_history.cap <= g_history.len)
        return 0;

    g_history.items[g_history.len] = linenoiseStrdup(line);
    if (g_history.items[g_history.len] == NULL)
        return 0;

    g_history.len += 1;

    while (g_history.len > g_history.maxLen)
    {
        free(g_history.items[0]);
        memmove(g_history.items, g_history.items + 1, sizeof(char *) * (size_t)(g_history.len - 1));
        g_history.len -= 1;
    }

    return 1;
}

void linenoiseRequestStop(void)
{
    InterlockedExchange(&g_stopRequested, 1);
}

void linenoiseResetStop(void)
{
    InterlockedExchange(&g_stopRequested, 0);
}

static int linenoiseIsStopRequested(void)
{
    return InterlockedCompareExchange(&g_stopRequested, 0, 0) != 0;
}

static void linenoiseWriteHint(const char *hint, size_t hintLen)
{
    HANDLE stdoutHandle;
    CONSOLE_SCREEN_BUFFER_INFO originalInfo;
    int hasColorConsole = 0;

    if (hint == NULL || hintLen == 0)
    {
        return;
    }

    stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (stdoutHandle != INVALID_HANDLE_VALUE && stdoutHandle != NULL)
    {
        if (GetConsoleScreenBufferInfo(stdoutHandle, &originalInfo))
        {
            hasColorConsole = 1;
            /* Draw predictive tail in dim gray, then restore original console colors. */
            SetConsoleTextAttribute(stdoutHandle, FOREGROUND_INTENSITY);
        }
    }

    fwrite(hint, 1, hintLen, stdout);

    if (hasColorConsole)
    {
        SetConsoleTextAttribute(stdoutHandle, originalInfo.wAttributes);
    }
}

static int linenoiseStartsWithIgnoreCase(const char *full, const char *prefix)
{
    while (*prefix != 0)
    {
        if (tolower((unsigned char)*full) != tolower((unsigned char)*prefix))
        {
            return 0;
        }
        ++full;
        ++prefix;
    }
    return 1;
}

static size_t linenoiseBuildHint(const char *buf, char *hint, size_t hintSize)
{
    linenoiseCompletions lc;
    size_t inputLen = 0;
    size_t i = 0;

    if (hint == NULL || hintSize == 0)
    {
        return 0;
    }
    hint[0] = 0;

    if (buf == NULL || buf[0] == 0 || g_completionCallback == NULL)
    {
        return 0;
    }

    lc.len = 0;
    lc.cvec = NULL;
    /* Reuse the completion callback and derive a "ghost text" suffix from the first extending match. */
    g_completionCallback(buf, &lc);

    inputLen = strlen(buf);
    for (i = 0; i < lc.len; ++i)
    {
        const char *candidate = lc.cvec[i];
        if (candidate == NULL)
        {
            continue;
        }
        if (strlen(candidate) <= inputLen)
        {
            continue;
        }
        if (!linenoiseStartsWithIgnoreCase(candidate, buf))
        {
            continue;
        }

        /* Keep only the part not yet typed by the user (rendered as hint text). */
        strncpy_s(hint, hintSize, candidate + inputLen, _TRUNCATE);
        break;
    }

    linenoiseClearCompletions(&lc);
    return strlen(hint);
}

static void linenoiseRedrawUnsafe(const char *prompt, const char *buf, int len, int pos, int *prevLen)
{
    int i;
    char hint[LINENOISE_MAX_LINE] = {0};
    int renderedLen = len;
    /* Hint length contributes to the rendered width so stale tail characters can be cleared correctly. */
    int hintLen = (int)linenoiseBuildHint(buf, hint, sizeof(hint));
    if (hintLen > 0)
    {
        renderedLen += hintLen;
    }

    fputc('\r', stdout);
    fputs(prompt, stdout);
    if (len > 0)
    {
        fwrite(buf, 1, (size_t)len, stdout);
    }
    if (hintLen > 0)
    {
        linenoiseWriteHint(hint, (size_t)hintLen);
    }

    if (*prevLen > renderedLen)
    {
        for (i = renderedLen; i < *prevLen; ++i)
        {
            fputc(' ', stdout);
        }
    }

    fputc('\r', stdout);
    fputs(prompt, stdout);
    if (pos > 0)
    {
        /* Cursor positioning reflects only real user input, not the ghost hint suffix. */
        fwrite(buf, 1, (size_t)pos, stdout);
    }

    fflush(stdout);
    *prevLen = renderedLen;
    linenoiseUpdateEditorState(prompt, buf, len, pos, *prevLen);
}

static void linenoiseRedraw(const char *prompt, const char *buf, int len, int pos, int *prevLen)
{
    linenoiseLockIo();
    linenoiseRedrawUnsafe(prompt, buf, len, pos, prevLen);
    linenoiseUnlockIo();
}

static int linenoiseStartsWith(const char *full, const char *prefix)
{
    while (*prefix != 0)
    {
        if (*full != *prefix)
            return 0;
        ++full;
        ++prefix;
    }
    return 1;
}

static int linenoiseComputeCommonPrefix(const linenoiseCompletions *lc, const char *seed, char *out, size_t outSize)
{
    size_t commonLen = 0;
    size_t i;

    if (lc->len == 0 || outSize == 0)
        return 0;

    strncpy_s(out, outSize, lc->cvec[0], _TRUNCATE);
    commonLen = strlen(out);

    for (i = 1; i < lc->len; ++i)
    {
        const char *candidate = lc->cvec[i];
        size_t j = 0;

        while (j < commonLen && out[j] != 0 && candidate[j] != 0 && out[j] == candidate[j])
            ++j;

        commonLen = j;
        out[commonLen] = 0;

        if (commonLen == 0)
            break;
    }

    if (strlen(out) <= strlen(seed))
        return 0;

    return linenoiseStartsWith(out, seed);
}

static void linenoiseApplyCompletion(const char *prompt, char *buf, int *len, int *pos, int *prevLen)
{
    linenoiseCompletions lc;
    int i;

    if (g_completionCallback == NULL)
    {
        Beep(750, 15);
        return;
    }

    lc.len = 0;
    lc.cvec = NULL;
    g_completionCallback(buf, &lc);

    if (lc.len == 0)
    {
        Beep(750, 15);
        linenoiseClearCompletions(&lc);
        return;
    }

    if (lc.len == 1)
    {
        strncpy_s(buf, LINENOISE_MAX_LINE, lc.cvec[0], _TRUNCATE);
        *len = (int)strlen(buf);
        *pos = *len;
        linenoiseRedraw(prompt, buf, *len, *pos, prevLen);
        linenoiseClearCompletions(&lc);
        return;
    }

    {
        char common[LINENOISE_MAX_LINE] = { 0 };
        if (linenoiseComputeCommonPrefix(&lc, buf, common, sizeof(common)))
        {
            strncpy_s(buf, LINENOISE_MAX_LINE, common, _TRUNCATE);
            *len = (int)strlen(buf);
            *pos = *len;
            linenoiseRedraw(prompt, buf, *len, *pos, prevLen);
        }
    }

    linenoiseLockIo();
    fputc('\n', stdout);
    for (i = 0; i < (int)lc.len; ++i)
    {
        fputs(lc.cvec[i], stdout);
        fputs("  ", stdout);
    }
    fputc('\n', stdout);
    linenoiseRedrawUnsafe(prompt, buf, *len, *pos, prevLen);
    linenoiseUnlockIo();
    linenoiseClearCompletions(&lc);
}

char *linenoise(const char *prompt)
{
    char buf[LINENOISE_MAX_LINE];
    int len = 0;
    int pos = 0;
    int prevLen = 0;
    int historyIndex = g_history.len;

    if (prompt == NULL)
        prompt = "";

    buf[0] = 0;
    linenoiseLockIo();
    linenoiseUpdateEditorState(prompt, buf, len, pos, prevLen);
    fputs(prompt, stdout);
    fflush(stdout);
    linenoiseUnlockIo();

    while (!linenoiseIsStopRequested())
    {
        if (!_kbhit())
        {
            Sleep(10);
            continue;
        }

        {
            int c = _getwch();

            if (c == 0 || c == 224)
            {
                int ext = _getwch();
                if (ext == 72)
                {
                    if (g_history.len > 0 && historyIndex > 0)
                    {
                        historyIndex -= 1;
                        strncpy_s(buf, sizeof(buf), g_history.items[historyIndex], _TRUNCATE);
                        len = (int)strlen(buf);
                        pos = len;
                        linenoiseRedraw(prompt, buf, len, pos, &prevLen);
                    }
                }
                else if (ext == 80)
                {
                    if (g_history.len > 0 && historyIndex < g_history.len)
                    {
                        historyIndex += 1;
                        if (historyIndex == g_history.len)
                            buf[0] = 0;
                        else
                            strncpy_s(buf, sizeof(buf), g_history.items[historyIndex], _TRUNCATE);

                        len = (int)strlen(buf);
                        pos = len;
                        linenoiseRedraw(prompt, buf, len, pos, &prevLen);
                    }
                }
                else if (ext == 75)
                {
                    if (pos > 0)
                    {
                        pos -= 1;
                        linenoiseRedraw(prompt, buf, len, pos, &prevLen);
                    }
                }
                else if (ext == 77)
                {
                    if (pos < len)
                    {
                        pos += 1;
                        linenoiseRedraw(prompt, buf, len, pos, &prevLen);
                    }
                }
                continue;
            }

            if (c == 3)
            {
                linenoiseLockIo();
                linenoiseDeactivateEditorState();
                fputc('\n', stdout);
                fflush(stdout);
                linenoiseUnlockIo();
                return NULL;
            }

            if (c == '\r' || c == '\n')
            {
                char *out;
                linenoiseLockIo();
                linenoiseDeactivateEditorState();
                fputc('\n', stdout);
                fflush(stdout);
                linenoiseUnlockIo();

                out = linenoiseStrdup(buf);
                return out;
            }

            if (c == '\t')
            {
                linenoiseApplyCompletion(prompt, buf, &len, &pos, &prevLen);
                continue;
            }

            if (c == 8)
            {
                if (pos > 0 && len > 0)
                {
                    memmove(buf + pos - 1, buf + pos, (size_t)(len - pos + 1));
                    pos -= 1;
                    len -= 1;
                    linenoiseRedraw(prompt, buf, len, pos, &prevLen);
                }
                continue;
            }

            if (isprint((unsigned char)c) && len < LINENOISE_MAX_LINE - 1)
            {
                if (pos == len)
                {
                    buf[pos++] = (char)c;
                    len += 1;
                    buf[len] = 0;
                }
                else
                {
                    memmove(buf + pos + 1, buf + pos, (size_t)(len - pos + 1));
                    buf[pos] = (char)c;
                    pos += 1;
                    len += 1;
                }
                linenoiseRedraw(prompt, buf, len, pos, &prevLen);
            }
        }
    }

    linenoiseLockIo();
    linenoiseDeactivateEditorState();
    fputc('\n', stdout);
    fflush(stdout);
    linenoiseUnlockIo();
    return NULL;
}

void linenoiseExternalWriteBegin(void)
{
    int i;
    int totalChars = 0;

    /* Lock shared console state and clear current prompt area before external output. */
    linenoiseLockIo();
    if (InterlockedCompareExchange(&g_editorActive, 0, 0) == 0)
    {
        return;
    }

    totalChars = (int)strlen(g_editorPrompt) + g_editorPrevLen;
    if (totalChars < 0)
    {
        totalChars = 0;
    }

    fputc('\r', stdout);
    for (i = 0; i < totalChars; ++i)
    {
        fputc(' ', stdout);
    }
    fputc('\r', stdout);
    fflush(stdout);
}

void linenoiseExternalWriteEnd(void)
{
    /* Restore prompt line after external output has been printed. */
    if (InterlockedCompareExchange(&g_editorActive, 0, 0) != 0)
    {
        int prevLen = g_editorPrevLen;
        linenoiseRedrawUnsafe(g_editorPrompt, g_editorBuf, g_editorLen, g_editorPos, &prevLen);
        g_editorPrevLen = prevLen;
    }
    linenoiseUnlockIo();
}
