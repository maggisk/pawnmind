#pragma once

#define WIN_SCORE (INT_MAX / 2)

void heuristics_init(void);
int evaluate_position(Player*, Player*);
