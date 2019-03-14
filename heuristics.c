#include <limits.h>
#include <assert.h>
#include "bithacks.h"
#include "movement.h"
#include "heuristics.h"
#include "constants.h"

#define PAWN_VALUE 20

int16_t PST[2][6][64];

static int16_t get_piece_value(Piece type) {
    switch (type) {
        case PAWN:   return 100;
        case KNIGHT: return 300;
        case BISHOP: return 300;
        case ROOK:   return 500;
        case QUEEN:  return 900;
        case KING:   return 6000;
    }
    assert(false);
    return -1;
}

static void set_PST(Piece type, int16_t *table) {
    for (int i = 0; i < 64; i++) {
        int64_t piece_value = get_piece_value(type);
        PST[WHITE][type][i] = piece_value + table[63 - i];
        PST[BLACK][type][i] = piece_value + table[i];
    }
}

void heuristics_init(void) {
    int16_t pawn_pst[] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25,  25, 10,  5,  5,
        0,  0,  0, 20,  20,  0,  0,  0,
        5, -5,-10,  0,   0,-10, -5,  5,
        5, 10, 10,-20, -20, 10, 10,  5,
        0,  0,  0,  0,   0,  0,  0,  0
    };

    int16_t knight_pst[] = {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    };

    int16_t bishop_pst[] = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    };

    int16_t rook_pst[] = {
        0,   0,  0,  0,  0,  0,  0,  0,
        5,  10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,   0,  0,  5,  5,  0,  0,  0
    };

    int16_t queen_pst[] = {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
        0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    };

    int16_t king_pst[] = {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        20, 20,  0,  0,  0,  0, 20, 20,
        20, 30, 10,  0,  0, 10, 30, 20
    };

    set_PST(PAWN, pawn_pst);
    set_PST(KNIGHT, knight_pst);
    set_PST(BISHOP, bishop_pst);
    set_PST(ROOK, rook_pst);
    set_PST(QUEEN, queen_pst);
    set_PST(KING, king_pst);
}

static int count_score(Player *p) {
    int score = 0;
    for (Piece type = FIRST_PIECE; type <= LAST_PIECE; type++)
        for (U64 pieces = p->pieces[type]; pieces;)
            score += PST[!p->is_white][type][lsb_index(pop_lsb(&pieces))];
    return score;
}

static U64 COLUMNS[] = {COL_A, COL_B, COL_C, COL_D, COL_E, COL_F, COL_G, COL_H, 0};

static int blocked_pawns(Player *me) {
    int score = 0, i = 0;
    for (U64 col; (col = COLUMNS[i]); i++) {
        int count = bitcount(me->pieces[PAWN] & col);
        if (count > 1)
            score -= PAWN_VALUE / 2 * (count - 1);
    }
    return score;
}

static int mobility_score(Player *me, Player *op) {
    int score = 0;

    for (Piece type = FIRST_PIECE; type <= LAST_PIECE; type++) {
        U64 pieces = me->pieces[type];
        while (pieces) {
            U64 from = pop_lsb(&pieces);
            U64 moves = get_moves(lsb_index(from), type, me, op);
            score += bitcount(moves & ~me->occupied) + 2 * bitcount(moves & op->occupied);
        }
    }

    return score;
}

static int evaluate_player(Player *me, Player *op) {
    return count_score(me) + mobility_score(me, op) + blocked_pawns(me);
}

int evaluate_position(Player *me, Player *op) {
    if (!me->pieces[KING])
        return -WIN_SCORE;

    if (!op->pieces[KING])
        return WIN_SCORE;

    return evaluate_player(me, op) - evaluate_player(op, me);
}
