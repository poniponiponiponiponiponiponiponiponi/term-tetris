#ifndef BLOCK_H
#define BLOCK_H

#include <stdbool.h>
#include <stddef.h>

#define BLOCK_ARR_DIM 4

typedef enum BlockType {
    BLOCK_EMPTY,
    BLOCK_I,
    BLOCK_J,
    BLOCK_L,
    BLOCK_O,
    BLOCK_S,
    BLOCK_T,
    BLOCK_Z,
    BLOCK_MAX
} BlockType;

typedef enum BlockColor {
    BLOCK_COLOR_BLACK,
    BLOCK_COLOR_RED,
    BLOCK_COLOR_GREEN,
    BLOCK_COLOR_YELLOW,
    BLOCK_COLOR_BLUE,
    BLOCK_COLOR_MAGENTA,
    BLOCK_COLOR_CYAN,
    BLOCK_COLOR_WHITE,
    BLOCK_COLOR_SHADOW
} BlockColor;

typedef enum Rotation {
    UP,
    RIGHT,
    DOWN,
    LEFT,
    ROTATION_MAX
} Rotation;

typedef struct Block {
    // top left x y
    int x;
    int y;
    Rotation rot;
    BlockType type;
} Block;

typedef struct Move {
    int x;
    int y;
} Move;

typedef struct SevenBag {
    size_t left;
    BlockType types[7];
} SevenBag;

BlockColor block_get_color(const BlockType type);
bool (*block_get_arr(const BlockType type))[BLOCK_ARR_DIM];
bool block_get_tile(const Block *const block, const int x, const int y);
Move block_get_rotation_offset(const BlockType type, const Rotation rot);
Move block_get_wallkick(const BlockType type, const Rotation rot, const int test);
SevenBag *seven_bag_create(void);
void seven_bag_fill(SevenBag *const bag);
void seven_bag_shuffle(SevenBag *const bag);
BlockType seven_bag_get(SevenBag *const bag);
void seven_bag_destroy(SevenBag *const bag);
int block_get_cell_amount(const BlockType type);

#endif
