#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <assert.h>
#include "transpos.h"
#include "bithacks.h"
#include "constants.h"

typedef struct TTNode {
    U64 hash;
    int value;
    struct TTNode *next;
    uint8_t depth_left;
} TTNode;

// node cache - we never malloc nodes
#define CACHE_SIZE (50*1000*1000)
static TTNode nodes[CACHE_SIZE];

// hash table
#define TABLE_SIZE (0xffffff)
static TTNode *hash_table[TABLE_SIZE];
static int current_index = 0;

// random hashes
#define N_HASHES (6 * 64 * 2)
static U64 hashes[N_HASHES];

void transpos_init(void) {
    FILE *fp = fopen("/dev/urandom", "r");
    if (!fp) {
        perror("Error reading from /dev/urandom\n");
        exit(-1);
    }

    for (int i = 0; i < N_HASHES; i++) {
        for (int j = 0; j < sizeof(hashes[0]); j++)
            hashes[i] = (hashes[i] << 8) | fgetc(fp);
    }

    fclose(fp);
}

U64 transpos_hash_square(Piece type, bool is_white, U64 sqr) {
    int start = is_white ? 0 : 6 * 64;
    int type_offset = 64 * type;
    int pos_offset = lsb_index(sqr);
    return hashes[start + type_offset + pos_offset];
}

U64 transpos_hash_player(Player *p) {
    U64 hash = 0;

    for (int i = FIRST_PIECE; i <= LAST_PIECE; i++) {
        U64 pieces = p->pieces[i];
        while (pieces)
            hash ^= transpos_hash_square(i, p->is_white, pop_lsb(&pieces));
    }

    return hash;
}

U64 transpos_hash_players(Player *us, Player *them) {
    return transpos_hash_player(us) ^ transpos_hash_player(them);
}

static void remove_node(TTNode *node) {
    int index = node->hash % TABLE_SIZE;
    TTNode *current = hash_table[index];
    if (current == node) {
        hash_table[index] = node->next;
    } else {
        while (current->next != node)
            current = current->next;
        current->next = node->next;
    }
}

static void insert_node(TTNode *node) {
    int index = node->hash % TABLE_SIZE;
    if (0 && hash_table[index] != NULL) {
        printf("hash table conflict\n");
        TTNode *n = hash_table[index];
        printf("%" PRIu64 "\n", node->hash);
        while (n) {
            printf("%" PRIu64 "\n", n->hash);
            n = n->next;
        }
    }
    node->next = hash_table[index];
    hash_table[index] = node;
}

static TTNode* get_new_node(void) {
    TTNode *node = &nodes[current_index];
    if (++current_index >= CACHE_SIZE) {
        current_index = 0;
        printf("transposition table circled\n");
    }
    if (node->hash)
        remove_node(node);
    return node;
}

void transpos_insert(U64 hash, bool is_white, int depth_left, int value) {
    if (!is_white)
        value = -value;

    // attempt to update existing node
    for (TTNode *node = hash_table[hash % TABLE_SIZE]; node; node = node->next) {
        if (node->hash == hash) {
            if (node->depth_left < depth_left) {
                node->depth_left = depth_left;
                node->value = value;
            }
            return;
        }
    }

    // otherwise insert a new one
    TTNode *node = get_new_node();
    node->hash = hash;
    node->value = value;
    node->depth_left = depth_left;
    insert_node(node);
}

bool transpos_lookup(U64 hash, bool is_white, int depth_left, int *value) {
    TTNode *node = hash_table[hash % TABLE_SIZE];

    while (node && node->hash != hash)
        node = node->next;

    if (!node)
        return false;

    if (node->depth_left < depth_left)
        return false;

    *value = node->value;
    if (!is_white)
        *value = -(*value);

    return true;
}
