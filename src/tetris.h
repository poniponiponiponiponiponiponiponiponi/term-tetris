#ifndef TETRIS_H
#define TETRIS_H

#include <ncurses.h>
#include <form.h>
#include <menu.h>

#include "window.h"
#include "board.h"
#include "circular_buffer.h"
#include "block.h"

typedef struct Stats {
    int rows;
    int blocks;
    int combo;
    int level;
    int score;
} Stats;

typedef struct HoldBox {
    bool swapped;
    BlockType curr_type; 
} HoldBox; 

typedef struct LockPieceDelay {
    int left_moves;
    int left_frames;
    int lowest;
} LockPieceDelay;

typedef enum State {
    STATE_NULL,
    STATE_TITLE,
    STATE_SINGLEPLAYER,
    STATE_MULTIPLAYER,
    STATE_SETTINGS,
    STATE_EXIT
} State;

extern State current_state;

typedef enum MultiState {
    MULTI_STATE_WAITING,
    MULTI_STATE_CONNECT,
    MULTI_STATE_PLAYING
} MultiState;

// This ctx struct is for board related things. In the case of multiplayer
// there will be multiple of those for each player and every board will be
// send to and received from server.
typedef struct BoardCtx {
    Board *board;
    CircularBuffer *buf;
    Block block;
    Stats stats;
    HoldBox hold;
    LockPieceDelay lock_piece_delay;
    bool block_out;
    bool lock_out;
} BoardCtx;

// This ctx struct is for rendering the BoardCtx.
typedef struct RenderCtx {
    Window *board_window;
    Window *buf_window;
    Window *stats_window;
    Window *hold_box_window;
    Window *block_delay_window;
} RenderCtx;

// This ctx struct is keeping the game state.
typedef struct GameCtx {
    SevenBag *bag;
    long long fps_counter;
    int rows2add;
    int move_ret;
    bool swap;
    BlockType to_swap;
    long long last_fall;
    bool quit;
} GameCtx;

typedef struct MultiCtx {
    GameCtx game_ctx;
    int socket;
    BoardCtx p1_board_ctx;
    RenderCtx p1_render_ctx;
    BoardCtx p2_board_ctx;
    RenderCtx p2_render_ctx;
    MultiState curr_multi_state;
} MultiCtx;

typedef enum MenuOptions {
    SINGLEPLAYER,
    MULTIPLAYER,
    SETTINGS,
    EXIT,
    MENU_OPTIONS_SIZE
} MenuOptions;

static const char *const menu_choices[] = {
    [SINGLEPLAYER] = "Singleplayer",
    [MULTIPLAYER] = "Multiplayer",
    [SETTINGS] = "Settings",
    [EXIT] = "Exit"
};

// generated with: $ figlet "tetris"
static const char *const tetris_logo =
" _____ _____ _____ ____  ___ ____\n"
"|_   _| ____|_   _|  _ \\|_ _/ ___|\n"
"  | | |  _|   | | | |_) || |\\___ \\\n"
"  | | | |___  | | |  _ < | | ___) |\n"
"  |_| |_____| |_| |_| \\_\\___|____/";

static const char menu_arrow[] = "--> ";

void init(void);
void uninit(void);
void singleplayer(void);
void init_singleplayer(BoardCtx *board, RenderCtx *render, GameCtx *game);
void uninit_singleplayer(BoardCtx *board, RenderCtx *render, GameCtx *game);
void singleplayer_input(BoardCtx *board, GameCtx *game);
void singleplayer_logic(BoardCtx *board, GameCtx *game);
void singleplayer_render(BoardCtx *board, RenderCtx *render);
void multiplayer(void);
void multiplayer_init(MultiCtx *ctx);
void multiplayer_uninit(MultiCtx *ctx);
void multiplayer_connect(MultiCtx *ctx);
void multiplayer_play(MultiCtx *ctx);
void title(void);

#endif
