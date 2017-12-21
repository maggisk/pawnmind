#pragma once
#include <stdbool.h>
#include <inttypes.h>
#include "constants.h"

void transpos_init(void);
U64 transpos_hash_players(Player*, Player*);
void transpos_insert(U64, bool, int, int);
bool transpos_lookup(U64, bool, int, int*);
