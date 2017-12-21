#pragma once
#include <stdbool.h>
#include "constants.h"

typedef struct {
    U64 from, to;
    bool ok;
} Move;

int search(Move*, Player*, Player*, int);
