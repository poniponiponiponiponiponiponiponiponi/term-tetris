#ifndef WINDOW_H
#define WINDOW_H

#include <curses.h>

typedef struct Window {
    WINDOW *win;
    int width;
    int height;
} Window;

Window *window_create(
        const int width,
        const int height,
        const int x,
        const int y);
void window_destroy(Window *window);

#endif
