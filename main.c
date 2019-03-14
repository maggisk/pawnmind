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
#include "heuristics.h"
#include "search.h"
#include "cli.h"
#include "transpos.h"
#include "util.h"
#include "proto_xboard.h"

Game* game_new(void) {
    Game *game = calloc(1, sizeof(*game));

    char *fen = parse_fen(FEN_INIT, &game->white, &game->black, NULL);
    if (*fen != '\0')
        printf("error parsing fen: '%s'\n", fen);

    return game;
}

int main(void) {
    // initialize
    srand(time(NULL));
    movement_init();
    heuristics_init();
    transpos_init();

    Game *game = game_new();

    cli_loop(&game->white, &game->black);
    //xboard_main();

    /*
    Move move;
    search(&move, &game->white, &game->black, 5);
    printf("%d %d (%d)\n", lsb_index(move.from), lsb_index(move.to), move.ok);
    */

    return 0;
}
