#pragma once
#include <inttypes.h>
#include <stdbool.h>

typedef uint64_t U64;

#define COL_A ((U64)0x8080808080808080)
#define COL_B (COL_A >> 1)
#define COL_C (COL_A >> 2)
#define COL_D (COL_A >> 3)
#define COL_E (COL_A >> 4)
#define COL_F (COL_A >> 5)
#define COL_G (COL_A >> 6)
#define COL_H (COL_A >> 7)

#define ROW_1 ((U64)0x00000000000000ff)
#define ROW_2 ((U64)0x000000000000ff00)
#define ROW_3 ((U64)0x0000000000ff0000)
#define ROW_4 ((U64)0x00000000ff000000)
#define ROW_5 ((U64)0x000000ff00000000)
#define ROW_6 ((U64)0x0000ff0000000000)
#define ROW_7 ((U64)0x00ff000000000000)
#define ROW_8 ((U64)0xff00000000000000)

#define FEN_INIT "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef enum {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
} Piece;

#define FIRST_PIECE PAWN
#define LAST_PIECE KING

typedef enum {
    WHITE,
    BLACK
} Color;

typedef struct {
    U64 pieces[6];
    U64 occupied, en_passant, castling, hash;
    bool is_white;
} Player;

typedef struct Game {
    Player white, black;
} Game;
