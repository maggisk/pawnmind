#include <string.h>
#include "util.h"

char* bbsqr_to_s(U64 sqr) {
    static char s[3];
    memset(s, 0, sizeof(s));

    for (int i = 0; i < 8; i++) {
        if ((COL_A >> i) & sqr)
            s[0] = 'a' + i;
        if ((ROW_1 << (i * 8)) & sqr)
            s[1] = '1' + i;
    }

    return s;
}
