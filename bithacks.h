#pragma once
#include "constants.h"

int bitcount(U64);
int bitcount_slow(U64);
U64 get_lsb(U64);
U64 clear_lsb(U64);
U64 pop_lsb(U64*);
int lsb_index(U64);
int lsb_index_slow(U64);
int msb_index(U64);
int msb_index_slow(U64);
