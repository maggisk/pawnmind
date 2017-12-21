#include <assert.h>
#include "constants.h"

int bitcount(U64 b) {
    return __builtin_popcountll(b);
}

int bitcount_slow(U64 b) {
    int count = 0;
    for (; b; b &= b - 1)
        count++;
    return count;
}

U64 get_lsb(U64 b) {
    return b & -b;
}

U64 clear_lsb(U64 b) {
    return b & (b - 1);
}

U64 pop_lsb(U64 *b) {
    U64 lsb = (*b) & -(*b);
    *b ^= lsb;
    return lsb;
}

int lsb_index(U64 b) {
    //return bitcount((b & -b) - 1);
    return __builtin_ctzll(b);
}

int lsb_index_slow(U64 b) {
    for (int i = 0; i < 64; i++)
        if ((b >> i) & 1)
            return i;
    return -1;
}

int msb_index(U64 b) {
    return 63 - __builtin_clzll(b);
}

int msb_index_slow(U64 b) {
    for (U64 i = b & (b - 1); i; i &= i - 1)
        b = i;
    return bitcount(b - 1);
}
