#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <time.h>
#include "bithacks.h"
#include "movement.h"
#include "search.h"

bool read_move(U64 *from, U64 *to) {
    char col_from, row_from, col_to, row_to;
    if (scanf("%c%c%c%c", &col_from, &row_from, &col_to, &row_to) < 0)
        return false;

    printf("read %c%c->%c%c\n", col_from, row_from, col_to, row_to);

    if (col_from < 'a' || col_from > 'h')
        return false;
    if (row_from < '1' || row_from > '8')
        return false;
    if (col_to < 'a' || col_to > 'h')
        return false;
    if (row_to < '1' || row_to > '8')
        return false;

    *from = (U64)1 << ((row_from - '1') * 8 + (7 - (col_from - 'a')));
    *to = (U64)1 << ((row_to - '1') * 8 + (7 - (col_to - 'a')));
    return true;
}

char* piece_to_string(Player *p, int i) {
    static char *piece_str[] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟", "♞", "♝", "♜", "♛", "♚"};
    U64 b = (U64)1 << i;

    for (int type = FIRST_PIECE; type <= LAST_PIECE; type++)
        if (b & p->pieces[type])
            return piece_str[type + 6 * (p->is_white ? 1 : 0)];
    return "";
}

static void game_print(Player *white, Player *black) {
    if (black->is_white) {
        Player *tmp = white;
        white = black;
        black = tmp;
    }

    for (int i = 63; i >= 0; i--) {
        if ((i+1) % 8 == 0)
            printf("\n%d ", (i / 8) + 1);
        char *s = piece_to_string(white, i);
        if (!s[0])
            s = piece_to_string(black, i);
        printf("%s ", s[0] ? s : "+");
    }
    printf("\n  A B C D E F G H\n");
}

/*
static void print_bitboard(U64 b, char c) {
    for (int i = 63; i >= 0; i--) {
        putchar((b >> i) & 1 ? c : '-');
        if (i % 8 == 0)
            putchar('\n');
    }
}
*/

void cli_loop(Player *me, Player *op) {
    game_print(me, op);

    if (me->pieces[KING] == 0 || op->pieces[KING] == 0) {
        printf("game over\n");
        return;
    }

    U64 from, to;
    Move move;

    printf("%s's turn\n", me->is_white ? "white" : "black");
    printf("> ");

    char command;
    do {
        scanf("%c", &command);
    } while (command == ' ' || command == '\n');

    switch (command) {
        case 'a':
            search(&move, me, op, 5);
            if (!apply_move_if_valid(move.from, move.to, me, op)) {
                printf("ai fucked up...\n");
                printf("exiting\n");
                return;
            }
            break;
        case 'm':
            while (!read_move(&from, &to))
                printf("can't read move...\n");
            if (!apply_move_if_valid(from, to, me, op)) {
                printf("can't make that move...\n");
                return cli_loop(me, op);
            }
            break;
        default:
            printf("garbage: %c\n", command);
            return cli_loop(me, op);
    }

    cli_loop(op, me);
}
