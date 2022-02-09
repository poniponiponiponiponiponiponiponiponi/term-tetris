#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "circular_buffer.h"

/* Circular buffer is in range [tail head). For example when there are three
 * elements in the buffer it looks like this:
 * - - - 1 2 3 - - -
 *       ^     ^
 *    tail     head
 * When the buffer is empty then tail == head.
 * When the buffer is full then also tail == head. */
CircularBuffer *buf_create(const size_t size) {
    CircularBuffer *ret = malloc(sizeof(CircularBuffer));
    if (ret == NULL) {
        fprintf(
                stderr,
                "Couldn't alloc circular buffer in function %s.\n",
                __func__);
        exit(EXIT_FAILURE);
    }
    ret->size = size;
    ret->used = 0;
    ret->buffer = malloc(sizeof(*ret->buffer) * size);
    if (ret == NULL) {
        fprintf(
                stderr,
                "Couldn't alloc circular buffer's buffer in function %s.\n",
                __func__);
        exit(EXIT_FAILURE);
    }
    ret->head = ret->buffer;
    ret->tail = ret->buffer;
    return ret;
}

void buf_destroy(CircularBuffer *const buf) {
    free(buf->buffer);
    free(buf);
}

int *buf_get_end(const CircularBuffer *const buf) {
    return buf->buffer + buf->size;
}

bool buf_is_full(const CircularBuffer *const buf) {
    return buf->size == buf->used;
}

int buf_add_head(CircularBuffer *const buf, const int val) {
    if (buf_is_full(buf)) {
        fprintf(stderr, "Circular buffer is full.\n");
        return -1;
    }
    buf->used++;
    *buf->head++ = val;
    if (buf->head == buf_get_end(buf))
        buf->head = buf->buffer;
    return 0;
}

int buf_add_tail(CircularBuffer *const buf, const int val) {
    if (buf_is_full(buf)) {
        fprintf(stderr, "Circular buffer is full.\n");
        return -1;
    }
    buf->used++;
    if (buf->tail == buf->buffer)
        buf->tail = buf_get_end(buf)-1;
    else
        buf->tail--;
    *buf->tail = val;
    return 0;
}

/* offset from 0 to used-1 */
int buf_get_head(const CircularBuffer *const buf, const int offset) {
    assert(offset >= 0);
    assert((size_t)offset < buf->used);

    const ptrdiff_t diff = buf->head - buf->buffer;
    if (diff > offset) {
        return *(buf->head-1 - offset);
    } else {
        const int left = offset - diff;
        return *(buf_get_end(buf)-1 - left);
    }
}

int buf_get_tail(const CircularBuffer *const buf, const int offset) {
    return buf_get_head(buf, buf->used-1-offset);
}

void buf_remove_head(CircularBuffer *const buf) {
    if (buf->used-- == 0) {
        fprintf(stderr, "buf is empty can remove head!\n");
        exit(EXIT_FAILURE);
    }
    if (buf->head == buf->buffer) {
        buf->head = buf_get_end(buf)-1;
    } else {
        buf->head--;
    }
}

void buf_remove_tail(CircularBuffer *const buf) {
    if (buf->used-- == 0) {
        fprintf(stderr, "buf is empty can remove head!\n");
        exit(EXIT_FAILURE);
    }
    if (buf->tail++ == buf_get_end(buf)) {
        buf->tail = buf->buffer;
    }
}
