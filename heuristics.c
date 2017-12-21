#include <limits.h>
#include "bithacks.h"
#include "movement.h"
#include "heuristics.h"

#define PAWN_VALUE 20

static int count_score(Player *p) {
    int score = bitcount(p->pieces[PAWN])
              + bitcount(p->pieces[KNIGHT]) * 3
              + bitcount(p->pieces[BISHOP]) * 3
              + bitcount(p->pieces[ROOK]) * 5
              + bitcount(p->pieces[QUEEN]) * 9;
    return score * PAWN_VALUE;
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
        return WIN_SCORE;

    if (!op->pieces[KING])
        return -WIN_SCORE;

    return evaluate_player(me, op) - evaluate_player(op, me);
}
