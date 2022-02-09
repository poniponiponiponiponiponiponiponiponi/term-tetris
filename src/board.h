#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>

#include "block.h"

typedef struct Board {
    int width;
    int height;
    /* Pointer to a memory allocation representing current state
     * where (blocks+x) + y*width is the currect blocks.
     * x, y = 0 is top left. x = width-1, y = height-1 is bottom right. */
    BlockType *blocks;
} Board;

Board *board_create(const int width, const int height);
void board_destroy(Board *const board);
BlockType *board_get_block(
        const Board *const board,
        const int x,
        const int y);
bool board_is_row_full(const Board *const board, const int y);
void board_erase_rows(Board *const board, const int from, const int to);
int board_clear_full_rows(Board *const board);

#endif
