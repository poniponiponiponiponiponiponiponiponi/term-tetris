#ifndef DEBUG_H
#define DEBUG_H

#include <stdlib.h>

#include "circular_buffer.h"
#include "window.h"

typedef struct Debug {
    char **lines;
    CircularBuffer *line_index_buf;
    size_t nolines;
    size_t max_lines;
    size_t max_line_length;
} Debug;

typedef struct DebugCtx {
    Window *debug_window;
    Debug *debug;
} DebugCtx;

extern DebugCtx *debug_ctx;


Debug *debug_create(const size_t size, const size_t max_line_len);
void debug_add_string(Debug *const debug, const char *s);
void debug(char *s, ...);
void show_debug(void);
void uninit_debug_ctx(void);
void init_debug_ctx(void);

#endif
