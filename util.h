#pragma once

#include "constants.h"

char* bbsqr_to_s(U64);
int coordinates_to_sqri(char *);
U64 coordinates_to_bitboard(char *);
char* parse_fen(char*, Player*, Player*, Color*);
