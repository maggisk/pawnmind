#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <time.h>
#include "constants.h"
#include "bithacks.h"
#include "movement.h"
#include "search.h"
#include "cli.h"
#include "transpos.h"

Game* game_new(void) {
    Game *game = calloc(1, sizeof(*game));

    Player *w = &game->white;
    w->pieces[PAWN] = 0xff00;
    w->pieces[ROOK] = 0b10000001;
    w->pieces[KNIGHT] = 0b1000010;
    w->pieces[BISHOP] = 0b100100;
    w->pieces[QUEEN] = 0b10000;
    w->pieces[KING] = 0b1000;
    w->is_white = true;
    w->occupied = 0xffff;
    w->castling = (COL_A | COL_H) & ROW_1;

    Player *b = &game->black;
    b->pieces[PAWN] = w->pieces[PAWN] << (8 * 5);
    b->pieces[ROOK] = w->pieces[ROOK] << (8 * 7);
    b->pieces[KNIGHT] = w->pieces[KNIGHT] << (8 * 7);
    b->pieces[BISHOP] = w->pieces[BISHOP] << (8 * 7);
    b->pieces[QUEEN] = w->pieces[QUEEN] << (8 * 7);
    b->pieces[KING] = w->pieces[KING] << (8 * 7);
    b->occupied = w->occupied << (8 * 6);
    b->castling = (COL_A | COL_H) & ROW_8;

    return game;
}

int main(void) {
    // initialize
    srand(time(NULL));
    movement_init();
    transpos_init();

    Game *game = game_new();

    cli_loop(&game->white, &game->black);

    /*
    Move move;
    search(&move, &game->white, &game->black, 5);
    printf("%d %d (%d)\n", lsb_index(move.from), lsb_index(move.to), move.ok);
    */

    return 0;
}
