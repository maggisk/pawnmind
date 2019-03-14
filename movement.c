#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include "movement.h"
#include "constants.h"
#include "bithacks.h"

static U64 move_north[64];
static U64 move_south[64];
static U64 move_east[64];
static U64 move_west[64];
static U64 move_north_west[64];
static U64 move_north_east[64];
static U64 move_south_west[64];
static U64 move_south_east[64];
static U64 move_bishop[64];
static U64 move_rook[64];
static U64 move_knight[64];
static U64 move_king[64];

static bool can_castle(Player *me, Player *op, int sqr, int rook_pos_diff, int dir) {
    // neither king nor rook may have moved
    if ((me->castling & ((U64)1 << (sqr + rook_pos_diff * dir))) == 0)
        return false;

    // squares between king and rook must be empty
    U64 occupied = me->occupied | op->occupied;
    for (int i = 1; i < rook_pos_diff; i++)
        if (((U64)1 << (sqr + i * dir)) & occupied)
            return false;

    // king must be safe, including squares he passes over
    for (int i = 0; i <= 2; i++)
        if (is_threatened(sqr + i * dir, me, op))
            return false;

    return true;
}

static bool can_castle_queenside(Player *me, Player *op, int sqr) {
    return can_castle(me, op, sqr, 4, 1);
}

static bool can_castle_kingside(Player *me, Player *op, int sqr) {
    return can_castle(me, op, sqr, 3, -1);
}

static int euclidean_squared(U64 a, U64 b) {
    int a_i = lsb_index(a);
    int b_i = lsb_index(b);
    int dist_x = (a_i % 8) - (b_i % 8);
    int dist_y = (a_i / 8) - (b_i / 8);
    return (dist_x * dist_x) + (dist_y * dist_y);
}

void movement_init(void) {
    U64 origin, sqr;

    for (int i = 0; i < 64; i++) {
        origin = (U64)1 << i;

        // fill rook move lookup table
        for (sqr = origin << 8; sqr; sqr <<= 8)
            move_north[i] |= sqr;
        for (sqr = origin >> 8; sqr; sqr >>= 8)
            move_south[i] |= sqr;
        for (sqr = origin << 1; sqr && (sqr & COL_H) == 0; sqr <<= 1)
            move_west[i] |= sqr;
        for (sqr = origin >> 1; sqr && (sqr & COL_A) == 0; sqr >>= 1)
            move_east[i] |= sqr;
        move_rook[i] = move_north[i] | move_west[i] | move_south[i] | move_east[i];

        // fill bishop move lookup table
        for (sqr = origin << 7; sqr && (sqr & COL_A) == 0; sqr <<= 7)
            move_north_east[i] |= sqr;
        for (sqr = origin << 9; sqr && (sqr & COL_H) == 0; sqr <<= 9)
            move_north_west[i] |= sqr;
        for (sqr = origin >> 7; sqr && (sqr & COL_H) == 0; sqr >>= 7)
            move_south_west[i] |= sqr;
        for (sqr = origin >> 9; sqr && (sqr & COL_A) == 0; sqr >>= 9)
            move_south_east[i] |= sqr;
        move_bishop[i] = move_north_east[i] | move_north_west[i] | move_south_west[i] | move_south_east[i];

        // fill knight move lookup table
        int knight_moves[] = {17, -17, 15, -15, 10, -10, 6, -6, 0};
        for (int m, j = 0; (m = knight_moves[j]); j++) {
            sqr = m > 0 ? (origin << m) : (origin >> -m);
            if (sqr && euclidean_squared(origin, sqr) == 5)
                move_knight[i] |= sqr;
        }

        // fill king move lookup table
        int king_moves[] = {9, 8, 7, 1, -1, -7, -8, -9, 0};
        for (int m, j = 0; (m = king_moves[j]); j++) {
            sqr = m > 0 ? (origin << m) : (origin >> -m);
            if (sqr && euclidean_squared(origin, sqr) <= 2)
                move_king[i] |= sqr;
        }
    }
}

bool is_threatened(int sqr, Player *me, Player *op) {
    U64 occupied = me->occupied | op->occupied;

    if (bishop_moves_all(sqr, occupied) & (op->pieces[BISHOP] | op->pieces[QUEEN]))
        return true;

    if (rook_moves_all(sqr, occupied) & (op->pieces[ROOK] | op->pieces[QUEEN]))
        return true;

    if (move_knight[sqr] & op->pieces[KNIGHT])
        return true;

    if (move_king[sqr] & op->pieces[KING])
        return true;

    // NB: we consider the square threatened by pawns even is there is no piece on
    // the square to be killed
    if (pawn_moves_all((U64)1 << sqr, occupied, me->is_white) & op->pieces[PAWN])
        return true;

    return false;
}

U64 rook_moves_all(int sqr, U64 occupied) {
    return move_rook[sqr]
         ^ move_north[lsb_index((move_north[sqr] & occupied) | ((U64)1 << 63))]
         ^ move_west[ lsb_index((move_west[sqr]  & occupied) | ((U64)1 << 63))]
         ^ move_south[msb_index((move_south[sqr] & occupied) | 1)]
         ^ move_east[ msb_index((move_east[sqr]  & occupied) | 1)];
}

U64 bishop_moves_all(int sqr, U64 occupied) {
    return move_bishop[sqr]
         ^ move_north_east[lsb_index((move_north_east[sqr] & occupied) | ((U64)1 << 63))]
         ^ move_north_west[lsb_index((move_north_west[sqr] & occupied) | ((U64)1 << 63))]
         ^ move_south_west[msb_index((move_south_west[sqr] & occupied) | 1)]
         ^ move_south_east[msb_index((move_south_east[sqr] & occupied) | 1)];
}

U64 queen_moves_all(int sqr, U64 occupied) {
    return rook_moves_all(sqr, occupied) | bishop_moves_all(sqr, occupied);
}

U64 knight_moves_all(int sqr) {
    return move_knight[sqr];
}

U64 king_moves_all(int sqr, Player *me, Player *op) {
    U64 moves = move_king[sqr];

    if (can_castle_queenside(me, op, sqr))
        moves |= (U64)1 << (sqr + 2);

    if (can_castle_kingside(me, op, sqr))
        moves |= (U64)1 << (sqr - 2);

    return moves;
}

// pawn move function is different because origin can contain multiple bits/pawns
// and the return bitboard will contain all squares any pawn can move to
// NB: occupied should include en passant when needed
U64 pawn_moves_all(U64 origin, U64 occupied, bool is_white) {
    U64 moves, empty = ~occupied;

    if (is_white) {
        moves  = (origin << 8) & empty;
        moves |= (moves  << 8) & empty & ROW_4;
        moves |= (origin << 9) & ~COL_H & occupied;
        moves |= (origin << 7) & ~COL_A & occupied;
    } else {
        moves  = (origin >> 8) & empty;
        moves |= (moves  >> 8) & empty & ROW_5;
        moves |= (origin >> 9) & ~COL_A & occupied;
        moves |= (origin >> 7) & ~COL_H & occupied;
    }

    return moves;
}

U64 get_moves(int sqr, Piece type, Player *me, Player *op) {
    switch (type) {
        case PAWN:
            return pawn_moves_all((U64)1 << sqr, me->occupied | op->occupied | op->en_passant, me->is_white);
        case KNIGHT:
            return knight_moves_all(sqr);
        case BISHOP:
            return bishop_moves_all(sqr, me->occupied | op->occupied);
        case ROOK:
            return rook_moves_all(sqr, me->occupied | op->occupied);
        case QUEEN:
            return queen_moves_all(sqr, me->occupied | op->occupied);
        case KING:
            return king_moves_all(sqr, me, op);
    }
    assert(false);
}

static void apply_move_simple(U64 from, U64 to, Piece type, Player *me) {
    me->pieces[type] ^= (from | to);
    me->occupied ^= (from | to);
}

void apply_move(U64 from, U64 to, Piece type, Player *me, Player *op) {
    assert((from & (me->pieces[type])) > 0);

    // update bitboards for the player that made the move
    apply_move_simple(from, to, type, me);

    // clear opponent square if it's a kill move
    if (op->occupied & to) {
        U64 not_sqr = ~to;
        op->occupied &= not_sqr;
        for (Piece p = FIRST_PIECE; p <= LAST_PIECE; p++)
            op->pieces[p] &= not_sqr;
    }

    // opponent can't castle with a rook that's been killed
    op->castling &= ~to;

    // reset previous en passant
    me->en_passant = 0;

    switch (type) {
        case KING:
            if (me->castling) {
                // can't castle after moving king
                me->castling = 0;

                // move rook when castling
                if (to & COL_G)
                    apply_move_simple(to >> 1, to << 1, ROOK, me);
                if (to & COL_C)
                    apply_move_simple(to << 2, to >> 1, ROOK, me);
            }
            break;

        case ROOK:
            // can't castle with a rook that's been moved
            me->castling &= ~from;
            break;

        case PAWN:
            // remove pawn if killed via en passant
            if (to == op->en_passant) {
                U64 not_sqr = ~(me->is_white ? (to >> 8) : (to << 8));
                assert(~not_sqr & op->pieces[PAWN]);
                op->occupied &= not_sqr;
                op->pieces[PAWN] &= not_sqr;
            }

            // keep track of square that can be killed via en passant by opponents next move
            if ((from & ROW_2) && (to & ROW_4))
                me->en_passant = from << 8;
            if ((from & ROW_7) && (to & ROW_5))
                me->en_passant = from >> 8;

            // pawn promotions
            if (to & (ROW_1 | ROW_8)) {
                me->pieces[PAWN] &= ~to;
                me->pieces[QUEEN] |= to;
            }

            break;

        default:
            break;
    }
}

bool apply_move_if_valid(U64 from, U64 to, Player *me, Player *op) {
    // invalid move if no piece at `from`
    if ((me->occupied & from) == 0)
        return false;

    // find the type of piece being moved
    Piece type;
    for (Piece t = FIRST_PIECE; t <= LAST_PIECE; t++)
        if (me->pieces[t] & from)
            type = t;

    // make sure we can move that piece to the requested square
    if ((get_moves(lsb_index(from), type, me, op) & to) == 0)
        return false;

    // it's valid - apply the move
    apply_move(from, to, type, me, op);
    return true;
}
