#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stddef.h>
#include <stdbool.h>

typedef struct CircularBuffer {
    size_t size;
    size_t used;
    int *buffer;
    int *head;
    int *tail;
} CircularBuffer;

CircularBuffer *buf_create(const size_t size);
void buf_destroy(CircularBuffer *const buf);
int *buf_get_end(const CircularBuffer *const buf);
bool buf_is_full(const CircularBuffer *const buf);
int buf_add_head(CircularBuffer *const buf, const int val);
int buf_add_tail(CircularBuffer *const buf, const int val);
int buf_get_head(const CircularBuffer *const buf, const int offset);
int buf_get_tail(const CircularBuffer *const buf, const int offset);
void buf_remove_head(CircularBuffer *const buf);
void buf_remove_tail(CircularBuffer *const buf);

#endif
