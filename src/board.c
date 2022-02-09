#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "board.h"

Board *board_create(const int width, const int height) {
    assert(width >= 1);
    assert(height >= 1);
    assert(width*height == (long long)width * (long long)height);

    Board *ret = (Board *)malloc(sizeof(Board));
    if (ret == NULL) {
        fprintf(stderr, "Couldn't alloc board in function %s.\n", __func__);
        exit(EXIT_FAILURE);
    }

    ret->width = width;
    ret->height = height;

    size_t blocks_size = width*height * sizeof(BlockType);
    ret->blocks = (BlockType *)malloc(blocks_size);
    if (ret->blocks == NULL) {
        fprintf(stderr,
                "Couldn't alloc board's blocks in function %s.\n",
                __func__);
        exit(EXIT_FAILURE);
    }
    memset(ret->blocks, BLOCK_EMPTY, blocks_size);

    return ret;
}

void board_destroy(Board *const board) {
    free(board->blocks);
    free(board);
}

BlockType *board_get_block(
        const Board *const board,
        const int x,
        const int y) {
    assert(x < board->width);
    assert(x >= 0);
    assert(y < board->height);
    assert(y >= 0);

    return board->blocks+x + y*board->width;
}

bool board_is_row_full(const Board *const board, const int y) {
    for (int x = 0; x < board->width; x++) {
        if (*board_get_block(board, x, y) == BLOCK_EMPTY)
            return false;
    }
    return true;
}

/* This function is for "shifting" blocks down when the blocks make a full row.
 * Variables 'from' and 'to' define a closed interval [from; to]
 * that will be erased and everything above will be shifted down.
 * Those range variables are defined in the same way y coordinate is
 * so 0 is top, height-1 is down. */
void board_erase_rows(Board *const board, const int from, const int to) {
    assert(from < board->height);
    assert(to < board->height);
    
    const int higher = to - from + 1;
    for (int y = to; y >= higher; y--) {
        for (int x = 0; x < board->width; x++) {
            if (y-higher < 0) {
                *board_get_block(board, x, y) = BLOCK_EMPTY;
            } else {
                *board_get_block(board, x, y) =
                    *board_get_block(board, x, y-higher);
            }
        }
    }
}

int board_clear_full_rows(Board *const board) {
    int removed_rows = 0;
    int from = -1;
    int to = -1;
    for (int y = 0; y < board->height; y++) {
        if (board_is_row_full(board, y)) {
            if (from == -1)
                from = to = y;
            else
                to++;
        } else {
            if (from != -1) {
                board_erase_rows(board, from, to);
                removed_rows += to - from + 1;
            }
            from = to = -1;
        }
    }
    if (from != -1){
        board_erase_rows(board, from, to);
        removed_rows += to - from + 1;
    }
    return removed_rows;
}
