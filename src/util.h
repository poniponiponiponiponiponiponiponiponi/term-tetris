#ifndef UTIL_H
#define UTIL_H

#include <ncurses.h>
#include <form.h>

#include "window.h"
#include "tetris.h"
#include "board.h"
#include "block.h"
#include "circular_buffer.h"
#include "debug.h"
#include "multiplayer.h"

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

#define FPS 60

#define FALL_AFTER_LEVEL1 48
#define FALL_AFTER_LEVEL2 43
#define FALL_AFTER_LEVEL3 38
#define FALL_AFTER_LEVEL4 33
#define FALL_AFTER_LEVEL5 28
#define FALL_AFTER_LEVEL6 23
#define FALL_AFTER_LEVEL7 18
#define FALL_AFTER_LEVEL8 13
#define FALL_AFTER_LEVEL9 8
#define FALL_AFTER_LEVEL10 6
#define FALL_AFTER_LEVEL11 5
#define FALL_AFTER_LEVEL14 4
#define FALL_AFTER_LEVEL17 3
#define FALL_AFTER_LEVEL20 2
#define FALL_AFTER_LEVEL30 1

#define BLOCK_HEIGHT 1
#define BLOCK_WIDTH 2

#define BOARD_HEIGHT 40
#define BOARD_WIDTH 10
#define FIRST_TRUE_ROW 20

#define LOCK_DEFAULT_MOVES 15
#define LOCK_DEFAULT_FRAMES 30
#define LOCK_DEFUALT_LOWEST 0

#define STD_BUF_SIZE 6

#define FIELD_SIZE 32

static const LockPieceDelay default_lock_piece_delay = {
    LOCK_DEFAULT_MOVES,
    LOCK_DEFAULT_FRAMES,
    LOCK_DEFUALT_LOWEST
};

Window *create_window_for_debug(
        const Debug *const debug,
        const int x,
        const int y);
Window *create_window_for_board(
        const Board *const board,
        const int x,
        const int y);
Window *create_window_for_buf(
        const CircularBuffer *const buf,
        const int x,
        const int y);
Window *create_window_for_stats(const int x, const int y);
Window *create_window_for_holdbox(const int x, const int y);
Window *create_window_for_block_delay(const int x, const int y);
int fall(Block *const block, Board *const board);
int bake(Block *const block, Board *const board);
// Those block functions should be in block.c/block.h instead but becuase
// they use both Block and Board in the declaration the compiler throws an
// error when I put them there. Its happening because of headers including
// each other.
bool block_can_move(
        const Block *const block,
        const Board *const board,
        const int mov_x,
        const int mov_y);
int block_move(
        Block *const block,
        const Board *const board,
        const int x,
        const int y);
int block_rotate_cw(Block *const block, const Board *const board);
int block_rotate_ccw(Block *const block, const Board *const board);
int block_wallkick(
        Block *const block,
        const Board *const board,
        const bool ccw);
Block cast_block_shadow(const Block *const block, const Board *const board);
bool is_block_on_ground(const Block *const block, const Board *const board);

void stats_update(Stats *const stats, const int cleared_rows);
int get_fall_after(const int level);
int get_level(const int score);
size_t get_logo_nolines(void);
size_t get_logo_length(void);
size_t get_menu_length(void);
int get_menu_option(
        const char *const selected_option,
        const char *const *const options,
        const int no_options);
void get_field_str(FIELD *const field, char *buf);
void send_board_ctx(const BoardCtx *const board_ctx, const int sockfd);


#endif
