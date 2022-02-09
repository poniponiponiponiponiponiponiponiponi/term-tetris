#include <curses.h>

#include "block.h"
#include "circular_buffer.h"
#include "debug.h"
#include "render.h"
#include "util.h"
#include "window.h"

void render_debug(
        Window *const window,
        const Debug *const debug
        ) {
    for (size_t i = 0; i < debug->nolines; i++) {
        int idx = buf_get_tail(debug->line_index_buf, i);
        mvwprintw(window->win, i, 1, "%s", debug->lines[idx]);
    }
}

void render_tile(
        Window *const window,
        const BlockColor color,
        const int x,
        const int y) {
    wattron(window->win, COLOR_PAIR(color));
    for (int wx = 0; wx < BLOCK_WIDTH; wx++) {
        for (int wy = 0; wy < BLOCK_HEIGHT; wy++) {
            mvwaddch(window->win,
                    y*BLOCK_HEIGHT + wy + 1,
                    x*BLOCK_WIDTH + wx + 1,
                    ':');
        }
    }
    wattroff(window->win, COLOR_PAIR(color));
}

void render_block(
        Window *const window,
        const Block *const block,
        const BlockColor color) {
    for (int x = 0; x < BLOCK_ARR_DIM; x++) {
        for (int y = 0; y < BLOCK_ARR_DIM; y++) {
            if (block_get_tile(block, x, y)) {
                render_tile(window, color, block->x+x, block->y+y);
            }
        }
    }
}

void render_board(Window *const window, const Board *const board) {
    box(window->win, 0, 0);
    for (int x = 0; x < board->width; x++) {
        for (int y = FIRST_TRUE_ROW; y < board->height; y++) {
            BlockType type = *board_get_block(board, x, y);
            if (type != BLOCK_EMPTY) {
                render_tile(window, block_get_color(type), x, y-FIRST_TRUE_ROW);
            }
        }
    }
}

void render_buf(Window *const window, const CircularBuffer *const buf) {
    box(window->win, 0, 0);
    for (size_t i = 0; i < buf->size; i++) {
        Block block = {
            .type = buf_get_head(buf, i),
            .x = 0,
            .y = BLOCK_ARR_DIM*i,
            .rot = UP
        };
        render_block(window, &block, block_get_color(block.type));
    }
}

void render_stats(Window *const window, const Stats *const stats) {
    box(window->win, 0, 0);
    mvwprintw(window->win, 1, 1, "rows: %d", stats->rows);
    mvwprintw(window->win, 2, 1, "blocks: %d", stats->blocks);
    mvwprintw(window->win, 3, 1, "combo: %d", stats->combo);
    mvwprintw(window->win, 4, 1, "level: %d", stats->level);
    mvwprintw(window->win, 5, 1, "score: %d", stats->score);
}

void render_hold_box(Window *const window, HoldBox *const hold) {
    box(window->win, 0, 0);
    Block block = {
        .type = hold->curr_type,
        .x = 0,
        .y = 0,
        .rot = UP
    };
    if (block.type != BLOCK_EMPTY)
        render_block(window, &block, block_get_color(block.type));
}

void render_block_delay(
        Window *const window,
        const LockPieceDelay *const block_delay) {
    box(window->win, 0, 0);
    for (int i = 0; i < block_delay->left_moves; i++)
        mvwaddch(window->win, 1, 2+i, '*');
    for (int i = 0; i < ((double)block_delay->left_frames/LOCK_DEFAULT_FRAMES)*15; i++)
        mvwaddch(window->win, 2, 2+i, '#');
}
