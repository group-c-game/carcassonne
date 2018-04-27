#include "board.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

size_t get_board_size_interactive() {
    size_t ret;
    while (true) {
        fputs("input board size: ", stdout);
        if (scanf("%lu", &ret) == 1) {
            // exhaust stdin
            for (int ch; (ch = getchar()) != EOF && ch != '\n' && ch != '\r';) { ; }
            break;
        }
        puts("bad input");
        // exhaust stdin
        for (int ch; (ch = getchar()) != EOF && ch != '\n' && ch != '\r';) { ; }
    }
    return ret;
}

size_t get_board_size(const char* filename) {
    FILE* file;
    if ((file = fopen(filename, "r")) == 0) {
        return false;
    }
    int ch;
    size_t row = 0, col = 0, col_max = 0;
    size_t count = 0;
    while ((ch = getc(file)) != EOF) {
        // increase columns on empty tile
        if (ch == '\t') {
            ++col;
            if (count) {
                return 0;
            }
            count = 0;
        }
        // increase rows on newline
        if (ch == '\n') {
            ++row;
            col_max = col > col_max ? col : col_max;
            col = 0;
            if (count) {
                return 0;
            }
            count = 0;
        }
        // ignore whitespace
        if (isspace(ch)) {
            continue;
        }
        ++count;
        if (count == 5) {
            count = 0;
            ++col;
        }
    }
    fclose(file);
    return row > col_max ? row : col_max;
}

board_t board_alloc(size_t size) {
    board_t board = malloc(sizeof(tile**) * size);
    for (size_t i = 0; i < size; ++i) {
        board[i] = malloc(sizeof(tile*) * size);
        memset(board[i], 0, sizeof(tile*) * size);
    }
    return board;
}

bool init_board(gamemode mode, const char* filename, sized_board* board) {
    if (mode == INTERACTIVE_NO_TILES || mode == INTERACTIVE) {
        board->size = get_board_size_interactive();
    } else {
        // margin
        board->size = get_board_size(filename) + 2;
    }
    board->fields = board_alloc(board->size);
    if (mode == AUTO && !parse_board(filename, board)) {
        return false;
    }
    return true;
}

sized_board init_board_exit_on_err(gamemode mode, const char* filename) {
    sized_board board;
    if (init_board(mode, filename, &board)) {
            fprintf(stderr, "error parsing board file: %s\n", filename);
            exit(EXIT_FAILURE);
    }
    return board;
}

void board_free(sized_board* board) {
    for (size_t i = 0; i < board->size; ++i) {
        for (size_t j = 0; j < board->size; ++j) {
            tile_free(board->fields[i][j]);
            free(board->fields[i][j]);
        }
        free(board->fields[i]);
    }
    free(board->fields);
    board->fields = 0;
}

bool board_is_empty(sized_board* board) {
    for (size_t i = 0; i < board->size; ++i) {
        for (size_t j = 0; j < board->size; ++j) {
            if (board->fields[i][j]) {
                return false;
            }
        }
    }
    return true;
}

bool can_place_tile(sized_board* board,
                    const tile* t, size_t y, size_t x) {
    // return false on null tile
    if (t == 0) {
        return false;
    }
    if (board_is_empty(board)) {
        return true;
    }
    // if out of bounds return false
    if (y > board->size || x > board->size) {
        return false;
    }
    // return false if target cell is already populated
    if (board->fields[y][x]) {
        return false;
    }
    // count amount of surrounding tiles
    int count = 0;
    // check if placement is on the edge and if neighbor tile is not null
    if (y != 0 && board->fields[y - 1][x]) {
        // increase amount of surrounding tiles count
        ++count;
        // check if the types are the same
        if (board->fields[y - 1][x]->down->type != t->up->type) {
            return false;
        }
    }
    if (y < board->size - 1 && board->fields[y + 1][x]) {
        ++count;
        if (board->fields[y + 1][x]->up->type != t->down->type) {
            return false;
        }
    }
    if (x != 0 && board->fields[y][x - 1]) {
        ++count;
        if (board->fields[y][x - 1]->right->type != t->left->type) {
            return false;
        }
    }
    if (x < board->size - 1 && board->fields[y][x + 1]) {
        ++count;
        if (board->fields[y][x + 1]->left->type != t->right->type) {
            return false;
        }
    }
    // return false if no surrounding tiles
    return count > 0;
}

rotation_t can_place_tile_rotated(sized_board* board,
                                  const tile* t, size_t y, size_t x) {
    tile temp = { t->up, t->right, t->down, t->left, t->mod };
    for (rotation_t rot = ROT_0; rot < ROT_NO; ++rot) {
        if (can_place_tile(board, tile_rotate_amount(rot, &temp), y, x)) {
            return rot;
        }
    }
    return ROT_NO;
}

void place_tile(tile** place, tile* t) {
    *place = t;
}

// TODO: take margin into consideration
bool parse_board(const char* filename, sized_board* board) {
    FILE* file;
    if ((file = fopen(filename, "r")) == 0) {
        return false;
    }
    int ch;
    char str[5];
    // i: rows, j: columns, count: current tile letter counter
    size_t i = 0, j = 0, count = 0;
    while ((ch = getc(file)) != EOF) {
        // increase columns on empty tile
        if (ch == '\t') {
            ++j;
            if (count != 0) {
                return false;
            }
            count = 0;
        }
        // increase rows on newline
        if (ch == '\n') {
            ++i;
            j = 0;
            if (count != 0) {
                return false;
            }
            count = 0;
        }
        // out of bounds
        if (i > board->size || j > board->size) {
            return false;
        }
        // ignore whitespace
        if (isspace(ch)) {
            continue;
        }
        // generate tile string and parse it, assign correct values to the board
        str[count++] = (char)ch;
        if (count == 5) {
            count = 0;
            tile_alloc_from_str(str, &(board->fields)[i][j]);
            ++j;
        }
    }
    fclose(file);
    return count == 0;
}

void print_board(sized_board* board) {
    // pass null tile pointer for can_place_tile to always fail
    print_board_legal_moves(board, 0);
}

void print_board_legal_moves(sized_board* board, tile* t) {
    // print rows
    for (size_t i = 0; i < board->size; ++i) {
        // print up
        for (size_t j = 0; j < board->size; ++j) {
            if (board->fields[i][j]) {
                printf("  %c  ", elem_to_char(board->fields[i][j]->up->type));
            } else {
                fputs("     ", stdout);
            }
            // print separator
            if (j < board->size - 1) {
                putchar('|');
            }
        }
        putchar('\n');
        // print mid
        for (size_t j = 0; j < board->size; ++j) {
            if (board->fields[i][j]) {
                printf(" %c%c%c ",
                       elem_to_char(board->fields[i][j]->left->type),
                       mod_to_char(board->fields[i][j]->mod),
                       elem_to_char(board->fields[i][j]->right->type));
            } else {
                printf("  %c  ", can_place_tile(board, t, i, j) ? 'x' : ' ');
            }
            if (j < board->size - 1) {
                putchar('|');
            }
        }
        putchar('\n');
        // print down
        for (size_t j = 0; j < board->size; ++j) {
            if (board->fields[i][j]) {
                printf("  %c  ", elem_to_char(board->fields[i][j]->down->type));
            } else {
                fputs("     ", stdout);
            }
            if (j < board->size - 1) {
                putchar('|');
            }
        }
        putchar('\n');
        // print separator
        if (i < board->size - 1) {
            for (size_t j = 1; j < board->size * 6; ++j) {
                putchar(j % 6 ? '-' : '+');
            }
        }
        putchar('\n');
    }
}

bool board_write(sized_board* board, const char* filename) {
    FILE* file;
    if ((file = fopen(filename, "w")) == 0) {
        return false;
    }
    char str[5];
    for (size_t i = 0; i < board->size; ++i) {
        for (size_t j = 0; j < board->size; ++j) {
            fprintf(file,
                    "%s ",
                    tile_to_str(board->fields[i][j], str));
        }
        fprintf(file, "\n");
    }
    fclose(file);
    return true;
}
