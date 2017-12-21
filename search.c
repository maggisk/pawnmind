#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "search.h"
#include "constants.h"
#include "bithacks.h"
#include "movement.h"
#include "heuristics.h"
#include "util.h"
#include "transpos.h"

static bool should_use_move(bool ignores_move, int new_score, int old_score, int *nscore) {
    // always if the score is improved
    if (new_score > old_score) {
        *nscore = 1; // we've now once chosen a score of this value
        return true;
    }

    // if the new score is worse don't use that move
    if (new_score < old_score)
        return false;

    // if we don't care about the actual move, only the score, we don't need to
    // waste time on remembering the move that gave this score
    if (ignores_move)
        return false;

    // otherwise we choose one of the best moves by random
    // this uses the same concept as reservous sampling
    return (rand() % ++(*nscore)) == 0;
}

int search(Move *move, Player *me, Player *op, int depth) {
    if (move != NULL)
        move->ok = false;

    int score = INT_MIN;
    int nscore = 0;
    U64 not_me = ~me->occupied;
    depth--;

    // loop through all types of pieces
    for (Piece type = FIRST_PIECE; type <= LAST_PIECE; type++) {
        U64 pieces = me->pieces[type];

        // loop through all pieces of this type
        while (pieces) {
            U64 from = pop_lsb(&pieces);
            U64 moves = not_me & get_moves(lsb_index(from), type, me, op);

            // loop through all possible moves
            while (moves) {
                U64 to = pop_lsb(&moves);

                // apply move
                Player me_copy = *me;
                Player op_copy = *op;
                apply_move(from, to, type, &me_copy, &op_copy);

                // check transposition table
                int new_score;
                U64 hash = transpos_hash_players(&me_copy, &op_copy);
                if (!transpos_lookup(hash, me->is_white, depth, &new_score)) {
                    // not in table, search deeper and insert new value
                    new_score = (depth <= 0)
                        ? evaluate_position(&me_copy, &op_copy)
                        : -search(NULL, &op_copy, &me_copy, depth);
                    transpos_insert(hash, me->is_white, depth, new_score);
                }

                // debug
                if (move != NULL) {
                    char c_from[3], c_to[3];
                    strcpy(c_from, bbsqr_to_s(from));
                    strcpy(c_to, bbsqr_to_s(to));
                    printf("score for %s -> %s : %d\n", c_from, c_to, new_score);
                }

                // remember move and score if it's better than we previously had
                if (should_use_move(move == NULL, new_score, score, &nscore)) {
                    score = new_score;
                    if (move != NULL) {
                        move->ok = true;
                        move->from = from;
                        move->to = to;
                    }
                }
            }
        }
    }

    return score;
}
