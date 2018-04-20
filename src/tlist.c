#include "tlist.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

bool tlist_parse(const char* filename, sized_tlist* list) {
    if (!list && !list->tiles) {
        return false;
    }
    FILE* file;
    if ((file = fopen(filename, "r")) == 0) {
        return false;
    }
    for (size_t i = 0; i < list->len; ++i) {
        // if any tile parsing fails return false
        if (!tile_parse(file, &list->tiles[i])) {
            fclose(file);
            return false;
        }
    }
    fclose(file);
    return true;
}

size_t tlist_get_len(const char* filename) {
    FILE* list = fopen(filename, "r");
    size_t count = 0;
    if (list) {
        int ch;
        while ((ch = getc(list)) != EOF) {
            while ((ch = getc(list)) != EOF && isspace(ch)) {
                continue;
            }
            if (ch != EOF && !isspace(ch)) {
                ++count;
            }
            while ((ch = getc(list)) != EOF && !isspace(ch)) {
                continue;
            }
        }
        fclose(list);
    }
    return count;
}

bool tlist_init(const char* filename, sized_tlist* list) {
    list->len = tlist_get_len(filename);
    list->tiles = malloc(sizeof(tile*) * list->len);
    memset(list->tiles, 0, sizeof(tile*) * list->len);
    return tlist_parse(filename, list);
}

sized_tlist tlist_init_exit_on_err(const char* filename) {
    sized_tlist list;
    if (!tlist_init(filename, &list)) {
        free(list.tiles);
        fputs("error parsing tile list\n", stderr);
        exit(EXIT_FAILURE);
    }
    return list;
}


void tlist_print(const sized_tlist* list) {
    for (size_t i = 0; i < list->len; ++i) {
        tile_print(list->tiles[i]);
        putchar('\n');
    }
}