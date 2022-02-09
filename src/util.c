#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "board.h"
#include "circular_buffer.h"
#include "debug.h"
#include "tetris.h"
#include "util.h"
#include "window.h"

void get_field_str(FIELD *const field, char *buf) {
    const char *s = field_buffer(field, 0);
    const char *end = s + FIELD_SIZE-1;

    while (isspace(*s))
        s++;
    while (s < end && isspace(*end))
        end--;
    while (s <= end)
        *buf++ = *s++;
    *buf = '\0';
}

Window *create_window_for_debug(
        const Debug *const debug,
        const int x,
        const int y) {
    return window_create(
            debug->max_line_length+2,
            debug->max_lines+2,
            x,
            y);
}

Window *create_window_for_board(
        const Board *const board,
        const int x,
        const int y) {
    return window_create(
            BLOCK_WIDTH*board->width + 2,
            BLOCK_HEIGHT*board->height + 2-20,
            x,
            y);
}

Window *create_window_for_buf(
        const CircularBuffer *const buf,
        const int x,
        const int y) {
    return window_create(
            BLOCK_WIDTH*BLOCK_ARR_DIM + 2,
            BLOCK_HEIGHT*buf->size*BLOCK_ARR_DIM + 2,
            x,
            y);
}

Window *create_window_for_stats(const int x, const int y) {
    return window_create(14+2, 5+2, x, y);
}

Window *create_window_for_holdbox(const int x, const int y) {
    return window_create(4*BLOCK_WIDTH+2, 4*BLOCK_HEIGHT+2, x, y);
}

Window *create_window_for_block_delay(const int x, const int y) {
    return window_create(17+2, 2+2, x, y);
}

int fall(Block *const block, Board *const board) {
    int rows = 0;
    int ret = block_move(block, board, 0, 1);
    if (ret != 0) {
        rows = bake(block, board);
        block->type = BLOCK_EMPTY;
    }
    return rows;
}

int bake(Block *const block, Board *const board) {
    for (int x = 0; x < BLOCK_ARR_DIM; x++) {
        for (int y = 0; y < BLOCK_ARR_DIM; y++) {
            if (block_get_tile(block, x, y)) {
                *board_get_block(board, block->x+x, block->y+y) = block->type;
            }
        }
    }
    return board_clear_full_rows(board);
}

bool block_can_move(
        const Block *const block,
        const Board *const board,
        const int mov_x,
        const int mov_y) {
    for (int tile_x = 0; tile_x < BLOCK_ARR_DIM; tile_x++) {
        for (int tile_y = 0; tile_y < BLOCK_ARR_DIM; tile_y++) {
            bool is_tile = block_get_tile(block, tile_x, tile_y);
            if (is_tile) {
                int x_pos = block->x + tile_x + mov_x;
                if (x_pos < 0)
                    return false;
                if (x_pos >= board->width)
                    return false;
                int y_pos = block->y + tile_y + mov_y;
                if (y_pos < 0)
                    return false;
                if (y_pos >= board->height)
                    return false;
                if (*board_get_block(board, x_pos, y_pos) != BLOCK_EMPTY)
                    return false;
            }
        }
    }
    return true;
}

int block_move(
        Block *const block,
        const Board *const board,
        const int x,
        const int y) {
    if (block_can_move(block, board, x, y)) {
        block->x += x;
        block->y += y;
        return 0;
    }
    return -1;
}

int block_rotate_cw(Block *const block, const Board *const board) {
    Rotation orig = block->rot;
    block->rot = (block->rot+1) % ROTATION_MAX;
    Move move = block_get_rotation_offset(block->type, block->rot);
    block->x += move.x;
    block->y += move.y;
    if (block_move(block, board, 0, 0) == 0) {
        return 0;
    } else if (block_wallkick(block, board, false) == 0) {
        return 0;
    } else {
        block->rot = orig;
        block->x -= move.x;
        block->y -= move.y;
        return -1;
    }
}

int block_rotate_ccw(Block *const block, const Board *const board) {
    Rotation orig = block->rot;
    if (block->rot == 0)
        block->rot = ROTATION_MAX-1;
    else
        block->rot = block->rot-1;
    Move move = block_get_rotation_offset(block->type, orig);
    block->x -= move.x;
    block->y -= move.y;
    if (block_move(block, board, 0, 0) == 0) {
        return 0;
    } else if (block_wallkick(block, board, true) == 0) {
        return 0;
    } else {
        block->rot = orig;
        block->x += move.x;
        block->y += move.y;
        return -1;
    }
}

int block_wallkick(
        Block *const block,
        const Board *const board,
        const bool ccw) {
    for (int i = 0; i < 4; i++) {
        Move move;
        if (ccw) {
            move = block_get_wallkick(
                    block->type,
                    (block->rot+1) % ROTATION_MAX,
                    i);
            move.x *= -1;
            move.y *= -1;
        } else {
            move = block_get_wallkick(block->type, block->rot, i);
        }
        if (block_move(block, board, move.x, move.y) == 0)
            return 0;
    }
    return -1;
}

Block cast_block_shadow(const Block *const block, const Board *const board) {
    Block shadow = *block;
    while (block_move(&shadow, board, 0, 1) == 0 && block->type != BLOCK_EMPTY)
        ;
    return shadow;
}

bool is_block_on_ground(const Block *const block, const Board *const board) {
    if (block->type == BLOCK_EMPTY)
        return false;
    return !block_can_move(block, board, 0, 1);
}

void stats_update(Stats *const stats, const int cleared_rows) {
    stats->rows += cleared_rows;
    switch (cleared_rows) {
        case 0:
            break;
        case 1:
            stats->score += 100 * stats->level;
            break;
        case 2:
            stats->score += 300 * stats->level;
            break;
        case 3:
            stats->score += 500 * stats->level;
            break;
        case 4:
            stats->score += 800 * stats->level;
            break;
        default:
            fprintf(stderr, "cleared rows weird number\n");
            exit(EXIT_FAILURE);
    }
    if (cleared_rows)
        stats->score += 50 * stats->combo * stats->level;
    stats->level = get_level(stats->score);
}

int get_fall_after(const int level) {
    if (level == 1)
        return FALL_AFTER_LEVEL1;
    if (level == 2)
        return FALL_AFTER_LEVEL2;
    if (level == 3)
        return FALL_AFTER_LEVEL3;
    if (level == 4)
        return FALL_AFTER_LEVEL4;
    if (level == 5)
        return FALL_AFTER_LEVEL5;
    if (level == 6)
        return FALL_AFTER_LEVEL6;
    if (level == 7)
        return FALL_AFTER_LEVEL7;
    if (level == 8)
        return FALL_AFTER_LEVEL8;
    if (level == 9)
        return FALL_AFTER_LEVEL9;
    if (level == 10)
        return FALL_AFTER_LEVEL10;
    if (level >= 11 && level <= 13)
        return FALL_AFTER_LEVEL11;
    if (level >= 14 && level <= 16)
        return FALL_AFTER_LEVEL14;
    if (level >= 17 && level <= 19)
        return FALL_AFTER_LEVEL17;
    if (level >= 20 && level <= 29)
        return FALL_AFTER_LEVEL20;
    if (level >= 30)
        return FALL_AFTER_LEVEL30;
    fprintf(stderr, "wrong level\n");
    exit(EXIT_FAILURE);
}

int get_level(const int score) {
    return score / 2500 + 1;
}

size_t get_logo_nolines(void) {
    size_t n = 1;
    for (size_t i = 0; i < strlen(tetris_logo); i++) {
        if (tetris_logo[i] == '\n')
            n++;
    }
    return n;
}

size_t get_logo_length(void) {
    size_t longest_line = 0;
    size_t curr_len = 0;
    for (size_t i = 0; i < strlen(tetris_logo); i++) {
        if (tetris_logo[i] == '\n') {
            if (longest_line < curr_len)
                longest_line = curr_len;
            curr_len = 0;
        } else {
            curr_len++;
        }
    }
    if (longest_line < curr_len)
        longest_line = curr_len;

    return longest_line;
}

size_t get_menu_length(void) {
    const size_t no_choices = ARRAY_SIZE(menu_choices);
    size_t longest = 0;
    for (size_t i = 0; i < no_choices; i++) {
        const char *const menu_choice = menu_choices[i];
        const size_t len = strlen(menu_choice);
        if (len > longest)
            longest = len;
    }
    return longest;
}

// Return index if selected option. If not found return -1.
int get_menu_option(
        const char *const selected_option,
        const char *const *const options,
        const int no_options) {
    for (int option = 0; option < no_options; option++) {
        if (strcmp(selected_option, options[option]) == 0)
            return option;
    }
    return -1;
}

const char *get_board_fmt(void) {
    static bool generated = false;
    /* fmt string:
     *
     * d - board width
     * d - board height
     * %ds - board memory where d is height * width
     *
     * ddd...d - all block types of a buf with std size
     *
     * d - block x
     * d - block y
     * d - block rot
     * d - block type
     *
     * d - stats rows
     * d - stats blocks
     * d - stats combo
     * d - stats level
     * d - stats score
     *
     * b - holdbox swapped
     * d - holdbox curr_type
     *
     * d - lockpiecedelay left_moves
     * d - lockpiecedelay left_frames
     * d - lockpiecedelay lowest
     *
     * b - block_out
     * b - lock_out
     */
    static char fmt[1024];

    if (!generated) {
        generated = true;
        // add board fmt
        sprintf(fmt, "dd%ds", BOARD_WIDTH*BOARD_HEIGHT);
        // add buf fmt
        for (size_t i = 0; i < STD_BUF_SIZE; i++)
            strcat(fmt, "d");
        // add block fmt
        strcat(fmt, "dddd");
        // add stats fmt
        strcat(fmt, "ddddd");
        // add holdbox fmt
        strcat(fmt, "bd");
        // add lockpiecedelay fmt
        strcat(fmt, "ddd");
        // add block_out and lock_out fmt
        strcat(fmt, "bb");
    }

    return fmt;
}

/* As long as there are less block types than 256 this will work fine...
 * and it makes the packing and unpacking much easier with the packing 
 * everything (numbers) as ascii character strings that I went with.
 * I don't really care about making packages as small as possible - even
 * without the compression the network usage would be around 600 KiB/s.
 * With compression the usage is around 6 KiB/s. */
void compress_board(const Board *const board, char *out) {
}

void send_board_ctx(const BoardCtx *const board_ctx, const int sockfd) {
    send_packet_type(sockfd, MULTI_UPDATE);
    const char *fmt = get_board_fmt();
    char *buf = calloc(1, fmt_length(fmt));
    pack(buf, fmt,
            board_ctx->board->width,
            board_ctx->board->height,
            board_ctx->board->blocks);
}
