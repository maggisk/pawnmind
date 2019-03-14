#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include "constants.h"
#include "util.h"
#include "search.h"
#include "movement.h"

/* xboard protocol implementation
 * https://www.gnu.org/software/xboard/engine-intf.html
 */

// state
Player white, black;
Color whose_turn;
bool force_mode = true;
#define ME (whose_turn == WHITE ? &white : &black)
#define OP (whose_turn == WHITE ? &black : &white)

__attribute__((unused))
static void debug_write(char *s) {
    FILE *f = fopen("xboard_debug.txt", "a");
    fputs(s, f);
    fclose(f);
}

static void cmd_protover(void) {
    char *version = strtok(NULL, " ");
    if (version == NULL || strtol(version, NULL, 10) < 2) {
        printf("Error (incompatible protocol version) %s\n", version);
        exit(-1);
    }

    printf("feature myname=\"PawnMind\"\n");
    printf("feature ping=1\n");
    printf("feature setboard=1\n");
    printf("feature sigint=0\n");
    printf("feature usermove=1\n");
    printf("feature draw=0\n");
    printf("feature sigint=0\n");
    printf("feature sigterm=0\n");
    printf("feature analyze=0\n");
    printf("feature debug=1\n");
    printf("feature colors=0\n");
    printf("feature done=1\n");
}

static void cmd_new(void) {
    force_mode = false;
    parse_fen(FEN_INIT, &white, &black, &whose_turn);
    printf("#%s\n", whose_turn == WHITE ? "white" : "black");
}

static void cmd_go(void) {
    force_mode = false;
    Move move;
    search(&move, ME, OP, 6);
    if (!apply_move_if_valid(move.from, move.to, ME, OP)) {
        printf("Error (ai fucked up)\n");
        exit(-1);
    }
    whose_turn = 1 - whose_turn;
    printf("move %s", bbsqr_to_s(move.from));
    printf("%s\n", bbsqr_to_s(move.to));
}

static void cmd_quit(void) {
    exit(0);
}

static void cmd_force(void) {
    force_mode = true;
}

static void cmd_ping(void) {
    char *n = strtok(NULL, " ");
    printf("pong %s\n", n);
}

static void cmd_usermove(void) {
    char *move = strtok(NULL, "");
    U64 from = coordinates_to_bitboard(move);
    U64 to = from == 0 ? 0 : coordinates_to_bitboard(move + 2);
    if (!from || !to || !apply_move_if_valid(from, to, ME, OP)) {
        printf("Illegal move: %s\n", move);
    } else {
        whose_turn = 1 - whose_turn;
        if (!force_mode)
            cmd_go();
    }
}

bool dispatch(char *handler_name, char *gui_command, void (*handler)(void)) {
    if (strcmp(gui_command, handler_name) == 0) {
        handler();
        return true;
    }
    return false;
}

static bool catch_all(char *cmd) {
    static char *ignores[] = {
        "xboard",
        "accepted",
        "rejected",
        "random",
        "hard",
        "easy",
        "?",
        NULL
    };

    for (int i = 0; ignores[i] != NULL; i++) {
        if (strcmp(cmd, ignores[i]) == 0) {
            printf("# ignored: %s %s\n", cmd, strtok(NULL, ""));
            return true;
        }
    }

    printf("Error (unknown command) %s\n", cmd);
    return true;
}

void xboard_main(void) {
    // make sure stdout is unbuffered
    setbuf(stdout, NULL);

    cmd_new();

    char *line = NULL;
    size_t buf_size = 0;
    ssize_t len;

    while ((len = getline(&line, &buf_size, stdin)) >= 0) {
        // debug_write(line);
        if (len && line[len - 1] == '\n')
            line[len - 1] = '\0';
        char *cmd = strtok(line, " ");

        (void)(dispatch("protover", cmd, cmd_protover)
            || dispatch("new", cmd, cmd_new)
            || dispatch("force", cmd, cmd_force)
            || dispatch("usermove", cmd, cmd_usermove)
            || dispatch("go", cmd, cmd_go)
            || dispatch("ping", cmd, cmd_ping)
            || dispatch("quit", cmd, cmd_quit)
            || catch_all(cmd));
    }
}
