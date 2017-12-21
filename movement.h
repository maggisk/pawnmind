#pragma once

#include <inttypes.h>
#include "constants.h"

void movement_init(void);
U64 rook_moves_all(int sqr, U64 occupied);
U64 bishop_moves_all(int sqr, U64 occupied);
U64 queen_moves_all(int sqr, U64 occupied);
U64 knight_moves_all(int sqr);
U64 king_moves_all(int sqr, Player*, Player*);
U64 pawn_moves_all(U64, U64, bool);
bool is_threatened(int, Player*, Player*);
U64 get_moves(int, Piece, Player*, Player*);
void apply_move(U64, U64, Piece, Player*, Player*);
bool apply_move_if_valid(U64, U64, Player*, Player*);
