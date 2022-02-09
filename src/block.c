#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "board.h"
#include "util.h"

static Move block_offset[4] = {
    { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }
};

static bool block_i_arr[BLOCK_ARR_DIM][BLOCK_ARR_DIM] = {
    { 0, 0, 0, 0 },
    { 1, 1, 1, 1 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
};

static bool block_j_arr[BLOCK_ARR_DIM][BLOCK_ARR_DIM] = {
    { 0, 0, 0, 0 },
    { 1, 0, 0, 0 },
    { 1, 1, 1, 0 },
    { 0, 0, 0, 0 },
};

static bool block_l_arr[BLOCK_ARR_DIM][BLOCK_ARR_DIM] = {
    { 0, 0, 0, 0 },
    { 0, 0, 1, 0 },
    { 1, 1, 1, 0 },
    { 0, 0, 0, 0 },
};

static bool block_o_arr[BLOCK_ARR_DIM][BLOCK_ARR_DIM] = {
    { 0, 0, 0, 0 },
    { 0, 1, 1, 0 },
    { 0, 1, 1, 0 },
    { 0, 0, 0, 0 },
};

static bool block_s_arr[BLOCK_ARR_DIM][BLOCK_ARR_DIM] = {
    { 0, 0, 0, 0 },
    { 0, 1, 1, 0 },
    { 1, 1, 0, 0 },
    { 0, 0, 0, 0 },
};

static bool block_t_arr[BLOCK_ARR_DIM][BLOCK_ARR_DIM] = {
    { 0, 0, 0, 0 },
    { 0, 1, 0, 0 },
    { 1, 1, 1, 0 },
    { 0, 0, 0, 0 },
};

static bool block_z_arr[BLOCK_ARR_DIM][BLOCK_ARR_DIM] = {
    { 0, 0, 0, 0 },
    { 1, 1, 0, 0 },
    { 0, 1, 1, 0 },
    { 0, 0, 0, 0 },
};

static bool block_empty_arr[BLOCK_ARR_DIM][BLOCK_ARR_DIM] = {
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
};

Move block_get_rotation_offset(const BlockType type, const Rotation rot) {
    switch (type) {
    case BLOCK_I:
    case BLOCK_O:
        return (Move){ 0, 0 };
    default:
        return block_offset[rot];
    }
}

Move block_get_wallkick(const BlockType type, const Rotation rot, const int test) {
    // table copied from:
    // https://tetris.fandom.com/wiki/SRS#SRS_vs_.5B.5BSRS.5D.5D
    switch (type) {
    case BLOCK_I:
        switch (rot) {
        // 0>>1
        case RIGHT:
            switch (test) {
            case 0:
                return (Move){ -2, 0 };
            case 1:
                return (Move){ 1, 0 };
            case 2:
                return (Move){ -2, 1 };
            case 3:
                return (Move){ 1, -2 };
            default:
                goto err;
            }
        // 1>>2
        case DOWN:
            switch (test) {
            case 0:
                return (Move){ -1, 0 };
            case 1:
                return (Move){ 2, 0 };
            case 2:
                return (Move){ -1, -2 };
            case 3:
                return (Move){ 2, 1 };
            default:
                goto err;
            }
        // 2>>3
        case LEFT:
            switch (test) {
            case 0:
                return (Move){ 2, 0 };
            case 1:
                return (Move){ -1, 0 };
            case 2:
                return (Move){ 2, -1 };
            case 3:
                return (Move){ -1, 2 };
            default:
                goto err;
            }
        // 3>>0
        case UP:
            switch (test) {
            case 0:
                return (Move){ 1, 0 };
            case 1:
                return (Move){ -2, 0 };
            case 2:
                return (Move){ 1, 2 };
            case 3:
                return (Move){ -2, -1 };
            default:
                goto err;
            }
        default:
            goto err;
        }
    default:
        switch (rot) {
        // 0>>1
        case RIGHT:
            switch (test) {
            case 0:
                return (Move){ -1, 0 };
            case 1:
                return (Move){ -1, -1 };
            case 2:
                return (Move){ 0, 2 };
            case 3:
                return (Move){ -1, 2 };
            default:
                goto err;
            }
        // 1>>2
        case DOWN:
            switch (test) {
            case 0:
                return (Move){ 1, 0 };
            case 1:
                return (Move){ 1, 1 };
            case 2:
                return (Move){ 0, -2 };
            case 3:
                return (Move){ 1, -2 };
            default:
                goto err;
            }
        // 2>>3
        case LEFT:
            switch (test) {
            case 0:
                return (Move){ 1, 0 };
            case 1:
                return (Move){ 1, -1 };
            case 2:
                return (Move){ 0, 2 };
            case 3:
                return (Move){ 1, 2 };
            default:
                goto err;
            }
        // 3>>0
        case UP:
            switch (test) {
            case 0:
                return (Move){ -1, 0 };
            case 1:
                return (Move){ -1, 1 };
            case 2:
                return (Move){ 0, -2 };
            case 3:
                return (Move){ -1, -2 };
            default:
                goto err;
            }
        default:
            goto err;
        }
    }
err:
    fprintf(stderr, "AAAAAAAAAAAAAAAAAAAAAA");
    exit(EXIT_FAILURE);
}

bool (*block_get_arr(const BlockType type))[BLOCK_ARR_DIM] {
    switch (type) {
    case BLOCK_I:
        return block_i_arr;
    case BLOCK_J:
        return block_j_arr;
    case BLOCK_L:
        return block_l_arr;
    case BLOCK_O:
        return block_o_arr;
    case BLOCK_S:
        return block_s_arr;
    case BLOCK_T:
        return block_t_arr;
    case BLOCK_Z:
        return block_z_arr;
    case BLOCK_EMPTY:
        return block_empty_arr;
    default:
        fprintf(stderr, "wrong block type in get_arr: %d\n", type);
        exit(EXIT_FAILURE);
    }
}

BlockColor block_get_color(const BlockType type) {
    switch (type) {
    case BLOCK_EMPTY:
    case BLOCK_I:
        return BLOCK_COLOR_CYAN;
    case BLOCK_J:
        return BLOCK_COLOR_BLUE;
    case BLOCK_L:
        return BLOCK_COLOR_YELLOW;
    case BLOCK_O:
        return BLOCK_COLOR_YELLOW;
    case BLOCK_S:
        return BLOCK_COLOR_GREEN;
    case BLOCK_T:
        return BLOCK_COLOR_MAGENTA;
    case BLOCK_Z:
        return BLOCK_COLOR_RED;
    default:
        fprintf(stderr, "wrong block type in get_color: %d\n", type);
        exit(EXIT_FAILURE);
    }
}

bool block_get_tile(const Block *const block, const int x, const int y) {
    assert(x < BLOCK_ARR_DIM);
    assert(x >= 0);
    assert(y < BLOCK_ARR_DIM);
    assert(y >= 0);

    bool (*arr)[BLOCK_ARR_DIM] = block_get_arr(block->type);
    switch (block->rot) {
    case UP:
        return arr[y][x];
    case RIGHT:
        return arr[BLOCK_ARR_DIM-x-1][y];
    case DOWN:
        return arr[BLOCK_ARR_DIM-y-1][BLOCK_ARR_DIM-x-1];
    case LEFT:
        return arr[x][BLOCK_ARR_DIM-y-1];
    default:
        fprintf(stderr, "wrong block rotation?");
        exit(EXIT_FAILURE);
    }
}

SevenBag *seven_bag_create(void) {
    SevenBag *ret = malloc(sizeof(SevenBag));
    if (ret == NULL) {
        fprintf(
                stderr,
                "Couldn't alloc seven bag in function %s.\n",
                __func__);
        exit(EXIT_FAILURE);
    }
    seven_bag_fill(ret);
    return ret;
}

void seven_bag_destroy(SevenBag *const bag) {
    free(bag);
}

void seven_bag_fill(SevenBag *const bag) {
    *bag = (SevenBag){ 7, { 1, 2, 3, 4, 5, 6, 7 } };
    seven_bag_shuffle(bag);
}

void seven_bag_shuffle(SevenBag *const bag) {
    // Fisher Yates shuffle
    for (size_t i = 7-1; i > 0; i--) {
        const size_t j = rand() % (i + 1);
        BlockType tmp = bag->types[j];
        bag->types[j] = bag->types[i];
        bag->types[i] = tmp;
    }
}

BlockType seven_bag_get(SevenBag *const bag) {
    if (bag->left <= 0)
        seven_bag_fill(bag);
    return bag->types[--bag->left];
}

int block_get_cell_amount(const BlockType type) {
    bool (*arr)[BLOCK_ARR_DIM] = block_get_arr(type);
    int cells = 0;
    for (int i = 0; i < BLOCK_ARR_DIM; i++) {
        for (int j = 0; j < BLOCK_ARR_DIM; j++) {
            if (arr[i][j])
                cells++;
        }
    }
    return cells;
}
