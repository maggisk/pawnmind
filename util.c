#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "util.h"
#include "constants.h"

char* bbsqr_to_s(U64 sqr) {
    static char s[3] = {0};

    for (int i = 0; i < 8; i++) {
        if ((COL_A >> i) & sqr)
            s[0] = 'a' + i;
        if ((ROW_1 << (i * 8)) & sqr)
            s[1] = '1' + i;
    }

    return s;
}

int char_to_piece(char c) {
    switch (tolower(c)) {
        case 'p': return PAWN;
        case 'n': return KNIGHT;
        case 'b': return BISHOP;
        case 'r': return ROOK;
        case 'q': return QUEEN;
        case 'k': return KING;
        default: return -1;
    }
}

int coordinates_to_sqri(char *sqr) {
    if (sqr[0] < 'a' || sqr[0] > 'h' || sqr[1] < '1' || sqr[1] > '8')
        return -1;

    return (sqr[1] - '1') * 8 + (7 - (sqr[0] - 'a'));
}

U64 coordinates_to_bitboard(char *sqr) {
    int i = coordinates_to_sqri(sqr);
    if (i == -1)
        return 0;

    return (U64)1 << i;
}

char* parse_fen(char *fen, Player *white_in, Player *black_in, Color *whose_turn_in) {
    Player white, black;
    memset(&white, 0, sizeof(white));
    memset(&black, 0, sizeof(black));
    white.is_white = true;

    // https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation

    // piece placement
    for (int sqr = 63; *fen && sqr >= 0; fen++) {
        if (*fen == '/')
            continue;
        if (*fen >= '0' && *fen <= '8') {
            sqr -= (*fen) - '0';
            continue;
        }

        int piece = char_to_piece(*fen);
        if (piece == -1)
            return fen;

        Player *p = (toupper(*fen) == *fen) ? &white : &black;
        p->pieces[piece] |= (U64)1 << sqr;
        p->occupied |= (U64)1 << sqr;
        sqr--;
    }

    // whitespace separator
    if (*fen != ' ')
        return fen;
    fen++;

    // active color
    if (*fen != 'w' && *fen != 'b')
        return fen;
    Color whose_turn = *fen == 'w' ? WHITE : BLACK;
    fen++;

    // whitespace separator
    if (*fen != ' ')
        return fen;
    fen++;

    // castling availability
    if (*fen == '-') {
        fen++;
    } else {
        while (*fen && *fen != ' ') {
            if (*fen == 'K')
                white.castling |= (ROW_1 & COL_H);
            else if (*fen == 'Q')
                white.castling |= (ROW_1 & COL_A);
            else if (*fen == 'k')
                black.castling |= (ROW_8 & COL_H);
            else if (*fen == 'q')
                black.castling |= (ROW_8 & COL_A);
            else
                return fen;
            fen++;
        }
    }

    // whitespace separator
    if (*fen != ' ')
        return fen;
    fen++;

    // en passant square
    if (*fen == '-') {
        fen++;
    } else {
        U64 sqr = coordinates_to_bitboard(fen);
        if (sqr == 0)
            return fen;
        fen += 2;
        if (whose_turn == WHITE)
            black.en_passant = sqr;
        else
            white.en_passant = sqr;
    }

    // whitespace separator
    if (*fen != ' ')
        return fen;
    fen++;

    // halfmove clock
    if (!isdigit(*fen))
        return fen;
    /*int halfmove_clock = */strtol(fen, &fen, 10);

    // whitespace separator
    if (*fen != ' ')
        return fen;
    fen++;

    // fullmoves
    if (!isdigit(*fen))
        return fen;
    /* int fullmoves = */strtol(fen, &fen, 10);

    // apply only if parsing was successful
    if (*fen == '\0') {
        *white_in = white;
        *black_in = black;
        if (whose_turn_in != NULL)
            *whose_turn_in = whose_turn;
    }

    return fen;
}
