#include <ncurses.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circular_buffer.h"
#include "debug.h"
#include "render.h"
#include "util.h"

DebugCtx *debug_ctx = NULL;

Debug *debug_create(const size_t size, const size_t max_line_len) {
    Debug *debug = malloc(sizeof(Debug));
    if (debug == NULL) {
        fprintf(stderr, "couldn't allocate debug in funcion %s.\n", __func__);
        exit(EXIT_FAILURE);
    }

    debug->max_lines = size;
    debug->max_line_length = max_line_len;
    debug->nolines = 0;

    debug->lines = malloc(sizeof(char *) * debug->max_lines);
    if (debug->lines == NULL) {
        fprintf(stderr,
                "couldn't allocate line ptrs in funcion %s.\n",
                __func__);
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < debug->max_lines; i++) {
        debug->lines[i] = malloc(sizeof(char) * debug->max_line_length);
        if (debug->lines[i] == NULL) {
            fprintf(stderr,
                    "couldn't allocate line in funcion %s.\n",
                    __func__);
            exit(EXIT_FAILURE);
        }
    }

    debug->line_index_buf = buf_create(debug->max_lines);
    
    return debug;
}

void debug_destroy(Debug *debug) {
    for (size_t i = 0; i < debug->max_lines; i++)
        free(debug->lines[i]);
    free(debug->lines);
    buf_destroy(debug->line_index_buf);
    free(debug);
}

void debug_add_string(Debug *const debug, const char *s) {
    int new_index;
    if (debug->nolines < debug->max_lines) {
        buf_add_head(debug->line_index_buf, debug->nolines);
        new_index = debug->nolines++;
    } else {
        new_index = buf_get_tail(debug->line_index_buf, 0);
        buf_remove_tail(debug->line_index_buf);
        buf_add_head(debug->line_index_buf, new_index);
    }

    strcpy(debug->lines[new_index], s);
}

void debug(char *s, ...) {
    if (debug_ctx == NULL)
        return;

    va_list args;
    va_start(args, s);

    size_t debug_str_len = debug_ctx->debug->max_line_length;
    char *debug_str = malloc(debug_str_len);
    if (debug_str == NULL) {
        fprintf(stderr, "debug str malloc error\n");
        exit(-1);
    }

    vsnprintf(debug_str, debug_str_len, s, args);
    debug_add_string(debug_ctx->debug, debug_str);
    free(debug_str);

    va_end(args);
}

void show_debug(void) {
    if (debug_ctx == NULL)
        return;
    werase(debug_ctx->debug_window->win);
    render_debug(debug_ctx->debug_window, debug_ctx->debug);
    wrefresh(debug_ctx->debug_window->win);
}

void init_debug_ctx(void) {
    debug_ctx = malloc(sizeof(DebugCtx));
    if (debug_ctx == NULL) {
        fprintf(stderr, "couldn't allocate debug_ctx\n");
        exit(-1);
    }

    debug_ctx->debug = debug_create(10, 80);
    debug_ctx->debug_window = create_window_for_debug(
            debug_ctx->debug, 20, 30);
}

void uninit_debug_ctx(void) {
    debug_destroy(debug_ctx->debug);
    window_destroy(debug_ctx->debug_window);
    free(debug_ctx);
}
