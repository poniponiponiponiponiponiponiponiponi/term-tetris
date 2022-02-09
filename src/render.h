#ifndef RENDER_H
#define RENDER_H

#include "tetris.h"
#include "block.h"
#include "window.h"
#include "board.h"
#include "circular_buffer.h"
#include "debug.h"

void render_block(
        Window *const window,
        const Block *const block,
        const BlockColor color);
void render_tile(
        Window *const window,
        const BlockColor color,
        const int x,
        const int y);
void render_board(Window *const window, const Board *const board);
void render_buf(Window *const window, const CircularBuffer *const buf);
void render_stats(Window *const window, const Stats *const stats);
void render_hold_box(Window *const window, HoldBox *const hold);
void render_block_delay(
        Window *const window,
        const LockPieceDelay *const block_delay);
void render_debug(
        Window *const window,
        const Debug *const debug);

#endif
