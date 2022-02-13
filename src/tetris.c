#include <ctype.h>
#include <form.h>
#include <locale.h>
#include <menu.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "block.h"
#include "board.h"
#include "circular_buffer.h"
#include "debug.h"
#include "multiplayer.h"
#include "render.h"
#include "tetris.h"
#include "util.h"
#include "window.h"

State current_state = STATE_TITLE;

int main(void) {
    init();

    while (current_state != STATE_EXIT) {
        switch (current_state) {
        case STATE_TITLE:
            title();
            break;
        case STATE_SINGLEPLAYER:
            singleplayer();
            break;
        case STATE_MULTIPLAYER:
            multiplayer();
            break;
        case STATE_SETTINGS:
            break;
        default:
            fprintf(stderr, "Weird state.\n");
            exit(EXIT_FAILURE);
        }
        clear();
    }

    uninit();

    return EXIT_SUCCESS;
}

void init(void) {
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();

    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    start_color();
    init_pair(BLOCK_COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
    init_pair(BLOCK_COLOR_RED, COLOR_RED, COLOR_RED);
    init_pair(BLOCK_COLOR_GREEN, COLOR_GREEN, COLOR_GREEN);
    init_pair(BLOCK_COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(BLOCK_COLOR_BLUE, COLOR_BLUE, COLOR_BLUE);
    init_pair(BLOCK_COLOR_MAGENTA, COLOR_MAGENTA, COLOR_MAGENTA);
    init_pair(BLOCK_COLOR_CYAN, COLOR_CYAN, COLOR_CYAN);
    init_pair(BLOCK_COLOR_WHITE, COLOR_WHITE, COLOR_WHITE);
    init_pair(BLOCK_COLOR_SHADOW, COLOR_WHITE, COLOR_BLACK);
    init_pair(69, COLOR_RED, COLOR_RED);

    srand(time(NULL));

    init_debug_ctx();
    debug("debug_ctx created");
}

void uninit(void) {
    uninit_debug_ctx();

    curs_set(1);
    endwin();
}

void singleplayer(void) {
    BoardCtx board;
    RenderCtx render;
    GameCtx game;

    init_singleplayer(&board, &render, &game);
    while (!game.quit) {
        singleplayer_input(&board, &game);
        singleplayer_logic(&board, &game);
        singleplayer_render(&board, &render);

        // After everything sleep... if we assume the program takes 0 zero
        // to process everything this will keep a constant frame rate.
        // This is bad but good enough for this simple tetris implementation.
        usleep(1000000/FPS);
    }
    uninit_singleplayer(&board, &render, &game);
}

void init_singleplayer(BoardCtx *board, RenderCtx *render, GameCtx *game) {
    *game = (GameCtx) {
        .bag = seven_bag_create(),
        .fps_counter = 0,
        .rows2add = 0,
        .move_ret = 0,
        .swap = false,
        .to_swap = BLOCK_EMPTY,
        .last_fall = 0,
        .quit = false
    };

    *board = (BoardCtx) {
        .board = board_create(BOARD_WIDTH, BOARD_HEIGHT),
        .buf = buf_create(STD_BUF_SIZE),
        .block = {
            // because the board row is of size 10 and blocks are of size 4
            // x = 3 is the middle of the board for blocks.
            .x = 3,
            .y = FIRST_TRUE_ROW,
            .rot = UP,
            .type = seven_bag_get(game->bag)
        },
        .stats = (Stats) {
            .rows = 0,
            .blocks = 0,
            .combo = 0,
            .level = 1,
            .score = 0
        },
        .hold = (HoldBox) { 
            .swapped = false,
            .curr_type = BLOCK_EMPTY
        },
        .lock_piece_delay = default_lock_piece_delay,
        .block_out = false,
        .lock_out = false
    };

    for (size_t i = 0; i < board->buf->size; i++)
        buf_add_head(board->buf, seven_bag_get(game->bag));

    *render = (RenderCtx) {
        .board_window = create_window_for_board(board->board, 6, 3),
        .buf_window = create_window_for_buf(board->buf, 30, 4),
        .stats_window = create_window_for_stats(39, 4),
        .hold_box_window = create_window_for_holdbox(39, 10),
        .block_delay_window = create_window_for_block_delay(6, 25),
    };
}

void uninit_singleplayer(BoardCtx *board, RenderCtx *render, GameCtx *game) {
    board_destroy(board->board);
    buf_destroy(board->buf);
    seven_bag_destroy(game->bag);
    window_destroy(render->board_window);
    window_destroy(render->buf_window);
    window_destroy(render->stats_window);
    window_destroy(render->hold_box_window);
    window_destroy(render->block_delay_window);
}

void singleplayer_input(BoardCtx *board, GameCtx *game) {
    int key = getch();
    switch (key) {
        case 'c':
        case 'C':
            if (!board->hold.swapped) {
                game->to_swap = board->hold.curr_type;
                board->hold.curr_type = board->block.type;
                board->hold.swapped = true;
                game->swap = true;
            }
            break;
        case KEY_LEFT:
            game->move_ret = block_move(&board->block, board->board, -1, 0);
            break;
        case KEY_RIGHT:
            game->move_ret = block_move(&board->block, board->board, 1, 0);
            break;
        case KEY_DOWN:
            game->rows2add += fall(&board->block, board->board);
            game->last_fall = game->fps_counter;
            break;
        case KEY_UP:
        case 'x':
        case 'X':
            game->move_ret = block_rotate_cw(&board->block, board->board);
            break;
        case 'z':
        case 'Z':
            game->move_ret = block_rotate_ccw(&board->block, board->board);
            break;
        case ' ':
            // hard drop points
            board->stats.score += 2 * block_get_cell_amount(board->block.type);
            board->block = cast_block_shadow(&board->block, board->board);
            game->rows2add += fall(&board->block, board->board);
            game->last_fall = game->fps_counter;
            break;
        case 'q':
        case 'Q':
            game->quit = true;
            break;
    }
}

void singleplayer_logic(BoardCtx *board, GameCtx *game) {
    // get the delta time that a block should fall after
    int fall_after = get_fall_after(board->stats.level);

    // if block is on ground use the lock piece delay left frames instead
    // of last fall and fps counter
    if (is_block_on_ground(&board->block, board->board)) {
        board->lock_piece_delay.left_frames--;
        game->last_fall = game->fps_counter;
    }

    // if block moved update lock piece delay
    if (!game->move_ret) {
        board->lock_piece_delay.left_frames = LOCK_DEFAULT_FRAMES;
        board->lock_piece_delay.left_moves--;
    }

    // if no more left frames or moves from piece delay then drop the piece
    if (board->lock_piece_delay.left_frames <= 0 || 
            (is_block_on_ground(&board->block, board->board) &&
             board->lock_piece_delay.left_moves <= 0)) {
        // a hack so the block would fall instantly
        game->last_fall = -fall_after;
        // soft drop points
        board->stats.score += 1 * block_get_cell_amount(board->block.type);
    }

    // drop block after certain time of not falling
    if (game->fps_counter >= game->last_fall+fall_after) {
        game->rows2add += fall(&board->block, board->board);
        game->last_fall = game->fps_counter;
    }

    // create (or get from swap) a new block
    if (board->block.type == BLOCK_EMPTY || game->swap) {
        game->last_fall = game->fps_counter;
        board->lock_piece_delay = default_lock_piece_delay;

        board->block = (Block){
                .x = 3,
                .y = FIRST_TRUE_ROW,
                .rot = UP,
                .type = game->swap ? game->to_swap : BLOCK_EMPTY
        };
        // get a new block from the buffer
        if (board->block.type == BLOCK_EMPTY) {
            board->block.type = buf_get_head(board->buf, 0);
            board->stats.blocks += 1;
            if (!game->swap)
                board->hold.swapped = false;
            buf_remove_head(board->buf);
            buf_add_tail(board->buf, seven_bag_get(game->bag));

            if (game->rows2add)
                board->stats.combo += 1;
            else
                board->stats.combo = 0;
        }

        // Check if can put a new block. If not the game is lost.
        board->block_out = true;
        for (int i = 0; i < 4; i++) {
            if (block_can_move(&board->block, board->board, 0, 0)) {
                board->block_out = false;
                break;
            }
            board->block.y += -1;
        }
    }

    // if there's a block in FIRST_TRUE_ROW-2 then game over.
    for (int i = 0; i < board->board->width; i++) {
        BlockType block = *board_get_block(board->board, i, FIRST_TRUE_ROW-2);
        if (block != BLOCK_EMPTY) {
            board->lock_out = true;
            break;
        }
    }

    // game over
    if (board->lock_out || board->block_out) {
        current_state = STATE_TITLE;
        game->quit = true;
        return;
    }

    // don't reset lock piece delay when the block was already on this height
    if (board->lock_piece_delay.lowest < board->block.y) {
        board->lock_piece_delay = default_lock_piece_delay;
        board->lock_piece_delay.lowest = board->block.y;
    }

    // update variables
    stats_update(&board->stats, game->rows2add);
    game->fps_counter++;
    game->rows2add = 0;
    game->swap = false;
    game->move_ret = -1;
}

void singleplayer_render(BoardCtx *board, RenderCtx *render) {
    werase(render->board_window->win);
    werase(render->buf_window->win);
    werase(render->stats_window->win);
    werase(render->hold_box_window->win);
    werase(render->block_delay_window->win);

    render_board(render->board_window, board->board);
    render_buf(render->buf_window, board->buf);
    render_stats(render->stats_window, &board->stats);
    render_hold_box(render->hold_box_window, &board->hold);
    render_block_delay(render->block_delay_window, &board->lock_piece_delay);

    // The +/- FIRST_TRUE_ROW below is a hack. Because the board is
    // of the size 10x40 tiles and only half of it is visible I render the
    // second half of the board as if it was the first half and I don't
    // render the first half (because it's invisible). Without the +/- part
    // the block would render 20 tiles below.
    Block shadow = cast_block_shadow(&board->block, board->board);
    shadow.y -= FIRST_TRUE_ROW;
    render_block(render->board_window, &shadow, BLOCK_COLOR_SHADOW);
    shadow.y += FIRST_TRUE_ROW;
    board->block.y -= FIRST_TRUE_ROW;
    render_block(
            render->board_window,
            &board->block,
            block_get_color(board->block.type));
    board->block.y += FIRST_TRUE_ROW;

    wrefresh(render->board_window->win);
    wrefresh(render->buf_window->win);
    wrefresh(render->stats_window->win);
    wrefresh(render->hold_box_window->win);
    wrefresh(render->block_delay_window->win);

    show_debug();
}

void multiplayer(void) {
    MultiCtx ctx;

    multiplayer_init(&ctx);

    while (1) {
        switch (ctx.curr_multi_state) {
        case MULTI_STATE_WAITING:
            printf("WAIT\n");
            int packett = recv_packet_type(ctx.socket);
            if (packett == MULTI_START) {
                ctx.curr_multi_state = MULTI_STATE_PLAYING;
            }
            printf("WAIT FINI\n");
            break;
        case MULTI_STATE_CONNECT:
            multiplayer_connect(&ctx);
            break;
        case MULTI_STATE_PLAYING:
            multiplayer_play(&ctx);
            if (ctx.game_ctx.quit) {
                current_state = STATE_TITLE;
                return;
            }
            break;
        default:
            fprintf(stderr, "multiplayer weird state\n");
            exit(EXIT_FAILURE);
        }
    }

    multiplayer_uninit(&ctx);
}

void multiplayer_init(MultiCtx *ctx) {
    ctx->curr_multi_state = MULTI_STATE_CONNECT;
    init_singleplayer(&ctx->p1_board_ctx, &ctx->p1_render_ctx, &ctx->game_ctx);

    ctx->p2_board_ctx = (BoardCtx) {
        .board = board_create(BOARD_WIDTH, BOARD_HEIGHT),
        .buf = buf_create(STD_BUF_SIZE)
    };
    for (size_t i = 0; i < ctx->p2_board_ctx.buf->size; i++)
        buf_add_head(ctx->p2_board_ctx.buf, BLOCK_EMPTY);

    ctx->p2_render_ctx = (RenderCtx) {
        .board_window = create_window_for_board(
                ctx->p2_board_ctx.board, 50+6, 3),
        .buf_window = create_window_for_buf(
                ctx->p2_board_ctx.buf, 50+30, 4),
        .stats_window = create_window_for_stats(50+39, 4),
        .hold_box_window = create_window_for_holdbox(50+39, 10),
        .block_delay_window = create_window_for_block_delay(50+6, 25)
    };
}

void multiplayer_uninit(MultiCtx *ctx) {
    uninit_singleplayer(
            &ctx->p1_board_ctx,
            &ctx->p1_render_ctx,
            &ctx->game_ctx);

    board_destroy(ctx->p2_board_ctx.board);
    buf_destroy(ctx->p2_board_ctx.buf);

    window_destroy(ctx->p2_render_ctx.board_window);
    window_destroy(ctx->p2_render_ctx.buf_window);
    window_destroy(ctx->p2_render_ctx.stats_window);
    window_destroy(ctx->p2_render_ctx.hold_box_window);
    window_destroy(ctx->p2_render_ctx.block_delay_window);
}

void multiplayer_connect(MultiCtx *ctx) {
    clear();
    curs_set(1);

    FIELD *fields[5];
    fields[0] = new_field(1, 5, 0, 0, 0, 0);
    fields[1] = new_field(1, FIELD_SIZE, 0, 6, 0, 0);
    fields[2] = new_field(1, 5, 2, 0, 0, 0);
    fields[3] = new_field(1, FIELD_SIZE, 2, 6, 0, 0);
    fields[4] = NULL;

    set_field_buffer(fields[0], 0, "ip:");
    set_field_buffer(fields[1], 0, "");
    set_field_buffer(fields[2], 0, "port:");
    set_field_buffer(fields[3], 0, "");

    set_field_opts(fields[0], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
    set_field_opts(fields[1], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);
    set_field_opts(fields[2], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
    set_field_opts(fields[3], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);

    for (size_t i = 0; i < ARRAY_SIZE(fields); i++)
        set_max_field(fields[i], FIELD_SIZE);

    set_field_back(fields[1], A_UNDERLINE);
    set_field_back(fields[3], A_UNDERLINE);

    FORM *form = new_form(fields);

    //set_form_win(form, stdscr);
    //set_form_sub(form, derwin(win_form, 18, 76, 1, 1));
    post_form(form);

    while (ctx->curr_multi_state == MULTI_STATE_CONNECT) {
        int c = getch();
        switch (c) {
        case KEY_DOWN:
            form_driver(form, REQ_NEXT_FIELD);
            break;
        case KEY_UP:
            form_driver(form, REQ_PREV_FIELD);
            break;
        case KEY_LEFT:
            form_driver(form, REQ_PREV_CHAR);
            break;
        case KEY_RIGHT:
            form_driver(form, REQ_NEXT_CHAR);
            break;
        case KEY_BACKSPACE:
            form_driver(form, REQ_DEL_PREV);
            break;
        case '\n':
            // need to do this for some reasons???
            form_driver(form, REQ_NEXT_FIELD);
            form_driver(form, REQ_PREV_FIELD);

            char ip[FIELD_SIZE+1];
            get_field_str(fields[1], ip);
            char port[FIELD_SIZE+1];
            get_field_str(fields[3], port);
            //mvprintw(10, 10, "%s", ip);
            int socket = get_client_socket(ip, port);
            if (socket != -1) {
                ctx->curr_multi_state = MULTI_STATE_WAITING;
            }
            ctx->socket = socket;
            break;
        default:
            form_driver(form, c);
            break;
        }
    }

    unpost_form(form);
    free_form(form);
    free_field(fields[0]);
    free_field(fields[1]);
    free_field(fields[2]);
    free_field(fields[3]);

    curs_set(0);
}

void recv_packet(MultiCtx *ctx) {
    PacketType packett = recv_packet_type(ctx->socket);
    switch (packett) {
        case MULTI_UPDATE:
            recv_board_ctx(&ctx->p2_board_ctx, ctx->socket);
            break;
        default:
            break;
    }
}

void multiplayer_play(MultiCtx *ctx) {
    debug("multiplayer: start");

    while (0) {
        debug("baka");

        send_packet_type(ctx->socket, MULTI_UPDATE);
        char fmt[] = "dd6sdbd";
        const size_t dst_len = fmt_length(fmt);
        char *dst = calloc(1, dst_len);

        pack(dst, fmt, 1, 2, "abcdef", 3, false, 4);
        sendall(ctx->socket, dst, dst_len);

        PacketType t = recv_packet_type(ctx->socket);
        if (t == MULTI_UPDATE)
            debug("multi update");
        else
            debug("WERID PACKET");

        int n1, n2, n3, n4;
        bool b1;
        char s1[7];

        recvall(ctx->socket, dst, dst_len);
        unpack(dst, fmt, &n1, &n2, s1, &n3, &b1, &n4);
        
        char output[123];
        sprintf(output, "%d %d %s %d %d %d", n1, n2, s1, n3, b1, n4);
        debug(output);

        free(dst);
        show_debug();
        usleep(1000000);
    }

    while (!ctx->game_ctx.quit) {
        singleplayer_input(&ctx->p1_board_ctx, &ctx->game_ctx);
        singleplayer_logic(&ctx->p1_board_ctx, &ctx->game_ctx);
        singleplayer_render(&ctx->p1_board_ctx, &ctx->p1_render_ctx);
        singleplayer_render(&ctx->p2_board_ctx, &ctx->p2_render_ctx);

        send_board_ctx(&ctx->p1_board_ctx, ctx->socket);
        recv_packet(ctx);

        usleep(1000000/FPS);
    }
}

void title(void) {
    // initialize variables
    const size_t no_choices = ARRAY_SIZE(menu_choices);

    ITEM **menu_items = calloc(no_choices+1, sizeof(ITEM *));
    for (size_t i = 0; i < no_choices; i++)
        menu_items[i] = new_item(menu_choices[i], NULL);
    menu_items[no_choices] = NULL;

    const size_t logo_length = get_logo_length();
    const size_t logo_nolines = get_logo_nolines();
    const size_t menu_length = get_menu_length();

    MENU *menu = new_menu(menu_items);
    WINDOW* menu_window = newwin(
            no_choices,
            menu_length + ARRAY_SIZE(menu_arrow),
            logo_nolines+1,
            logo_length/2 - menu_length/2 - ARRAY_SIZE(menu_arrow)
            );
    set_menu_win(menu, menu_window);
    set_menu_sub(menu, menu_window);
    set_menu_mark(menu, menu_arrow);
    post_menu(menu);

    printw(tetris_logo);

    while (current_state == STATE_TITLE) {
        // input
        int c = getch();
        switch (c) {
        case KEY_DOWN:
            menu_driver(menu, REQ_DOWN_ITEM);
            break;
        case KEY_UP:
            menu_driver(menu, REQ_UP_ITEM);
            break;
        case KEY_RIGHT:
        case ' ':
        case '\n':
            // Empty statement below because the c language is weird:
            // https://stackoverflow.com/questions/18496282/why-do-i-get-a-label-can-only-be-part-of-a-statement-and-a-declaration-is-not-a.
            // Gcc allows it and only prints a warning with -Wpedantic
            // but clang throws an error there, lol.
            ;
            int selected = get_menu_option(
                    item_name(current_item(menu)),
                    menu_choices,
                    no_choices);
            switch (selected) {
            case SINGLEPLAYER:
                current_state = STATE_SINGLEPLAYER;
                break;
            case MULTIPLAYER:
                current_state = STATE_MULTIPLAYER;
                break;
            case SETTINGS:
                current_state = STATE_SETTINGS;
                break;
            case EXIT:
                current_state = STATE_EXIT;
                break;
            }

            break;
        }
        
        // rendering
        show_debug();
        wrefresh(menu_window);
    }

    unpost_menu(menu);
    free_menu(menu);
    for (size_t i = 0; i < no_choices; i++)
        free_item(menu_items[i]);
    free(menu_items);
}
