#ifndef TILE_H
#define TILE_H

#include <stdbool.h>
#include <stddef.h>

// define type representing single tile
typedef enum { CASTLE, ROAD, FIELD } element;
typedef enum { NONE, SHIELD, TEMPLE } modifier;
typedef struct {
    element up;
    element right;
    element down;
    element left;
    modifier mod;
} tile;

#define TILE_LIST_LEN 30
extern tile* g_tile_list[TILE_LIST_LEN];    // list of aviable tiles

tile parse_tile();                          // parse single tile
bool parse_tile_list();                     // populate g_tile_list

char element_to_char(element);
char modifier_to_char(modifier);
// probably avoid calling this directly or make sure buff is at least 5 cells long
char* tile_to_str(const tile*, char*);
void print_tile(const tile*);
void print_tile_list(const tile*, size_t);

bool write_tile_list(const tile*, size_t);

// gameplay
int rate_tile(const tile*);                 // return a tile rating
tile choose_best_tile(const tile*, size_t); // choose best tile from the list based on rating

// rotate clockwise tile supplied by pointer,
// return pointer for checking validity or chaining function calls
// if pointer is null only return it
tile* rotate_tile(tile*);

#endif
