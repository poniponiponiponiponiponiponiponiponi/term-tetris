#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "window.h"

Window *window_create(
        const int width,
        const int height,
        const int x,
        const int y) {
    assert(width >= 1);
    assert(height >= 1);
    assert(width*height == (long long)width * (long long)height);

    Window *ret = malloc(sizeof(Window));
    if (ret == NULL) {
        fprintf(stderr, "Couldn't alloc window in function %s.\n", __func__);
        exit(EXIT_FAILURE);
    }

    ret->win = newwin(height, width, y, x);
    if (ret->win == NULL) {
        fprintf(stderr,
                "Couldn't create window's win in function %s.\n",
                __func__);
        exit(EXIT_FAILURE);
    }

    ret->width = width;
    ret->height = height;

    return ret;
}

void window_destroy(Window *window) {
    delwin(window->win);
    free(window);
}
