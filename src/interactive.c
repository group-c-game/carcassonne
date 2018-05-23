#include "interactive.h"

#include "board.h"
#include "tlist.h"
#include "calculator.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void greeting() {
    puts("hello player!\n"
         "welcome to a simple carcassonne based game!\n"
         "for usage run: carcassonne help\n"
         "for help type '?'\n");
}

void usage() {
    puts("usage: carcassonne [tiles-list-file] [board-file]\n"
         "tiles-list-file and board-file should be flies in current directory\n"
         "if both tiles-list-file and board-file specified run in auto mode\n"
         "if only tiles-list given use list specified in interactive mode\n"
         "if none file specified use default tile list for interactive mode\n");
}

void init_tlist_interactive(sized_tlist* list) {
    free(list->tiles);
    list->tiles = 0;
    char name[64] = { 0 };
    while (true) {
        fputs("enter name of a file containing tile list: ", stdout);
        fgets(name, sizeof(name), stdin);
        name[strcspn(name, "\n")] = '\0';
        if (tlist_init(name, list)) {
            return;
        }
        fputs("initializing failed, try again.\n", stderr);
    }
}

void write_tlist_interactive(sized_tlist* list) {
    char filename[64] = { 0 };
    fputs("enter name to save list as: ", stdout);
    fgets(filename, ARR_LEN(filename), stdin);
    filename[strcspn(filename, "\n")] = '\0';
    tlist_write(list, filename);
}

void load_board_interactive(sized_board* board) {
    board_free(board);
    board->tiles = 0;
    char name[64] = { 0 };
    while (true) {
        fputs("enter name of a file containing board: ", stdout);
        fgets(name, sizeof(name), stdin);
        name[strcspn(name, "\n")] = '\0';
        // mode auto to load board from file
        if (board_init(AUTO, name, board)) {
            return;
        }
        fputs("initializing failed, try again.\n", stderr);
    }
}

void write_board_interactive(sized_board* board) {
    char filename[64] = { 0 };
    fputs("enter name to save board as: ", stdout);
    fgets(filename, ARR_LEN(filename), stdin);
    filename[strcspn(filename, "\n")] = '\0';
    board_write(board, filename);
}

tile* choose_tile_interactive(sized_tlist* list, tile** t) {
    tile* temp = *t;
    fputs("choose tile (number): ", stdout);
    size_t choice;
    while (true) {
        scanf("%lu", &choice);
        // exhaust stdin
        for (int ch; (ch = getchar()) != EOF && ch != '\n' && ch != '\r';) { ; }
        if (choice == 0 || choice <= list->size) {
            break;
        }
        puts("choice out of bounds");
    }
    // choose right tile based on user input (numbering from 1 and ignore null pointers)
    for (size_t i = 0, count = 0; i < list->size; ++i) {
        if (list->tiles[i] && ++count == choice) {
            *t = list->tiles[i];
            list->tiles[i] = 0;
        }
    }
    // if current tile is not null put it back on the list
    if (temp) {
        // find empty space
        for (size_t i = 0; i < list->size; ++i) {
            // TODO: if it wont find empty space it will leak memory
            if (list->tiles[i] == 0) {
                list->tiles[i] = temp;
                break;
            }
        }
    }
    return temp;
}

void rotate_tile_interactive(tile** t) {
    rotation_t rot;
    while (true) {
        fputs("rotate by: (0) 0 deg, (1) 90 deg, (2) 180 deg, (3) 270 deg: ",
              stdout);
        if (scanf("%d", (int*)&rot) && rot >= 0 && rot <= 3) {
            tile_rotate_amount(rot, *t);
            for (int ch; (ch = getchar()) != EOF && ch != '\n' && ch != '\r';) { ; }
            return;
        }
        for (int ch; (ch = getchar()) != EOF && ch != '\n' && ch != '\r';) { ; }
        fputs("bad input\n", stderr);
    }
}

// zmień x y na h w
// numerowanie od 1 nie 0
// naprawic info o rotacji
// jakaś niezła imba daje rzeczy których nie ma
void place_tile_interactive(sized_board* board, sized_tlist* list, tile** t) {
    if (*t == 0) {
        choose_tile_interactive(list, t);
    }
    size_t h, w;
    while (true) {
        fputs("where to place tile (h w): ", stdout);
        if (scanf("%lu %lu", &h, &w) == 2) {
            for (int ch; (ch = getchar()) != EOF && ch != '\n' && ch != '\r';) { ; }
            if (h > board->size || w > board->size) {
                fputs("out of bounds\n", stderr);
                continue;
            }
            // czy to x y jest dobrze?
            if (tile_can_place(board, *t, h, w)) {
                tile_place(&board->tiles[h][w], *t);
                *t = 0;
                return;
            }
            //rotation_t rot;
            //if ((rot = can_place_tile_rotated(board, *t, h, w))) {
            //    fputs("tile can't be placed right now "
            //          "but could be placed if it was rotated by ", stdout);
            //    rotation_print(rot);
            //    putchar('\n');
            //    return;
            //}
            fputs("can't place tile here\n", stderr);
        } else {
            for (int ch; (ch = getchar()) != EOF && ch != '\n' && ch != '\r';) { ; }
            fputs("bad input\n", stderr);
        }
    }
}

typedef struct {
    sized_tlist* list;
    sized_board* board;
    tile* c_tile;
} state;

#define UNUSED(x) (void)x
#define MAKE_STATE_FUNC(f) bool f ## _state(state* s) {         \
        assert(s);                                              \
        f();                                                    \
        UNUSED(s);                                              \
        return true; }

#define MAKE_STATE_FUNC_LIST(f) bool f ## _state(state* s) {    \
        assert(s);                                              \
        f(s->list);                                             \
        return true; }

#define MAKE_STATE_FUNC_BOARD(f) bool f ## _state(state* s) {   \
        assert(s);                                              \
        f(s->board);                                            \
        return true; }

MAKE_STATE_FUNC(greeting)
MAKE_STATE_FUNC(usage)
MAKE_STATE_FUNC(help)
MAKE_STATE_FUNC_LIST(tlist_print)
MAKE_STATE_FUNC_LIST(init_tlist_interactive)
MAKE_STATE_FUNC_LIST(write_tlist_interactive)
MAKE_STATE_FUNC_BOARD(board_print)
MAKE_STATE_FUNC_BOARD(load_board_interactive)
MAKE_STATE_FUNC_BOARD(write_board_interactive)

bool choose_tile_interactive_state(state* s) {
    choose_tile_interactive(s->list, &s->c_tile);
    return true;
}

bool tile_print_state(state* s) {
    tile_print(s->c_tile);
    putchar('\n');
    return true;
}

bool rotate_tile_interactive_state(state* s) {
    rotate_tile_interactive(&s->c_tile);
    return true;
}

bool board_print_legal_moves_state(state* s) {
    board_print_legal_moves(s->board, s->c_tile);
    return true;
}

bool place_tile_interactive_state(state* s) {
    place_tile_interactive(s->board, s->list, &s->c_tile);
    return true;
}

void change_prompt();
MAKE_STATE_FUNC(change_prompt)

bool score_interactive_state(state* s) {
    printf("current score is: %d\n", score(s->board));
    return true;
}

bool quit_state(state* s) {
    UNUSED(s);
    return false;
}

// command enumerator, user command, command description
// if you want to abbrevietions and not have them printed in help command
// put them right after main command
// (only first command with specific enum value is printed)
const struct { const char* cmd; const char* desc; bool (*func)(state*); } act_list[] = {
    { "greeting",     "greets player",                  greeting_state                  },
    { "g",            "abbrev",                         greeting_state                  },
    { "usage",        "prints usage",                   usage_state                     },
    { "u",            "abbrev",                         usage_state                     },
    { "help",         "prints this message",            help_state                      },
    { "h",            "abbrev",                         help_state                      },
    { "?",            "abbrev",                         help_state                      },
    { "print list",   "prints tile list",               tlist_print_state               },
    { "p l",          "abbrev",                         tlist_print_state               },
    { "load list",    "load tile list file",            init_tlist_interactive_state    },
    { "l l",          "abbrev",                         init_tlist_interactive_state    },
    { "write list",   "write list to file",             write_tlist_interactive_state   },
    { "print board",  "prints the board",               board_print_state               },
    { "p b",          "abbrev",                         board_print_state               },
    { "load board",   "load board file",                load_board_interactive_state    },
    { "l b",          "abbrev",                         load_board_interactive_state    },
    { "write board",  "write board to file",            write_board_interactive_state   },
    { "choose tile",  "choose tile to place",           choose_tile_interactive_state   },
    { "c t",          "abbrev",                         choose_tile_interactive_state   },
    { "print tile",   "print current tile",             tile_print_state                },
    { "rotate tile",  "rotate current tile",            rotate_tile_interactive_state   },
    { "print moves",  "print moves aviable",            board_print_legal_moves_state   },
    { "p m",          "abbrev",                         board_print_legal_moves_state   },
    { "place tile",   "place choosen tile",             place_tile_interactive_state    },
    { "prompt",       "change prompt text",             change_prompt_state             },
    { "score",        "give score for current board",   score_interactive_state         },
    { "quit",         "quits the game",                 quit_state                      },
    { "q",            "abbrev",                         quit_state                      },
    { "exit",         "abbrev",                         quit_state                      },
    { "e",            "abbrev",                         quit_state                      },
};

void help() {
    for (size_t i = 0; i < ARR_LEN(act_list); ++i) {
        // do not print commands marked as abbreviations
        // prints if desc different than 'abbrev'
        // (strcmp returns 0 if the same)
        if (!STR_EQ(act_list[i].desc, "abbrev")) {
            printf("%s: %s\n", act_list[i].cmd, act_list[i].desc);
        }
    }
}

char prompt[32] = "> ";

void change_prompt() {
    fputs("new prompt: ", stdout);
    fgets(prompt, sizeof(prompt), stdin);
    prompt[strcspn(prompt, "\n")] = '\0';
}

bool run_prompt(state* s) {
    fputs(prompt, stdout);

    char input[32] = { 0 };
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0';

    for (size_t i = 0; i < ARR_LEN(act_list); ++i) {
        if (STR_EQ(input, act_list[i].cmd)) {
            return act_list[i].func(s);
        }
    }
    return false;
}

void run_interactive(gamemode mode, const char* list_filename) {
    greeting();
    sized_tlist list;
    if (!tlist_init(list_filename, &list)) {
        init_tlist_interactive(&list);
    }

    sized_board board;
    if (!board_init(mode, 0, &board)) {
        load_board_interactive(&board);
    }

    tile* c_tile = 0;

    state s = { &list, &board, c_tile };

    while (run_prompt(&s)) { ; }

    tlist_free(&list);
    board_free(&board);
    tile_free(c_tile);
    free(c_tile);
}
