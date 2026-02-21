#include "keymap.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Key name to SDL_Scancode mapping
typedef struct {
    const char* name;
    SDL_Scancode code;
} KeyNameEntry;

static const KeyNameEntry KEY_NAME_TABLE[] = {
    // Letters
    {"a", SDL_SCANCODE_A}, {"b", SDL_SCANCODE_B}, {"c", SDL_SCANCODE_C},
    {"d", SDL_SCANCODE_D}, {"e", SDL_SCANCODE_E}, {"f", SDL_SCANCODE_F},
    {"g", SDL_SCANCODE_G}, {"h", SDL_SCANCODE_H}, {"i", SDL_SCANCODE_I},
    {"j", SDL_SCANCODE_J}, {"k", SDL_SCANCODE_K}, {"l", SDL_SCANCODE_L},
    {"m", SDL_SCANCODE_M}, {"n", SDL_SCANCODE_N}, {"o", SDL_SCANCODE_O},
    {"p", SDL_SCANCODE_P}, {"q", SDL_SCANCODE_Q}, {"r", SDL_SCANCODE_R},
    {"s", SDL_SCANCODE_S}, {"t", SDL_SCANCODE_T}, {"u", SDL_SCANCODE_U},
    {"v", SDL_SCANCODE_V}, {"w", SDL_SCANCODE_W}, {"x", SDL_SCANCODE_X},
    {"y", SDL_SCANCODE_Y}, {"z", SDL_SCANCODE_Z},
    // Numbers
    {"0", SDL_SCANCODE_0}, {"1", SDL_SCANCODE_1}, {"2", SDL_SCANCODE_2},
    {"3", SDL_SCANCODE_3}, {"4", SDL_SCANCODE_4}, {"5", SDL_SCANCODE_5},
    {"6", SDL_SCANCODE_6}, {"7", SDL_SCANCODE_7}, {"8", SDL_SCANCODE_8},
    {"9", SDL_SCANCODE_9},
    // Special keys
    {"space", SDL_SCANCODE_SPACE},
    {"escape", SDL_SCANCODE_ESCAPE},
    {"return", SDL_SCANCODE_RETURN},
    {"enter", SDL_SCANCODE_RETURN},
    {"tab", SDL_SCANCODE_TAB},
    {"backspace", SDL_SCANCODE_BACKSPACE},
    // Arrow keys
    {"up", SDL_SCANCODE_UP},
    {"down", SDL_SCANCODE_DOWN},
    {"left", SDL_SCANCODE_LEFT},
    {"right", SDL_SCANCODE_RIGHT},
    // Modifiers
    {"lshift", SDL_SCANCODE_LSHIFT},
    {"rshift", SDL_SCANCODE_RSHIFT},
    {"lctrl", SDL_SCANCODE_LCTRL},
    {"rctrl", SDL_SCANCODE_RCTRL},
    {"lalt", SDL_SCANCODE_LALT},
    {"ralt", SDL_SCANCODE_RALT},
    {NULL, 0}
};

// Action name to KeyAction mapping
typedef struct {
    const char* name;
    KeyAction action;
} ActionNameEntry;

static const ActionNameEntry ACTION_NAME_TABLE[] = {
    {"p1_move_up",    KEY_P1_MOVE_UP},
    {"p1_move_down",  KEY_P1_MOVE_DOWN},
    {"p1_move_left",  KEY_P1_MOVE_LEFT},
    {"p1_move_right", KEY_P1_MOVE_RIGHT},
    {"p1_shoot",      KEY_P1_SHOOT},
    {"p2_move_up",    KEY_P2_MOVE_UP},
    {"p2_move_down",  KEY_P2_MOVE_DOWN},
    {"p2_move_left",  KEY_P2_MOVE_LEFT},
    {"p2_move_right", KEY_P2_MOVE_RIGHT},
    {"p2_shoot",      KEY_P2_SHOOT},
    {"screenshot",    KEY_SCREENSHOT},
    {"pause",         KEY_PAUSE},
    {"quit",          KEY_QUIT},
    {NULL, 0}
};

static SDL_Scancode key_name_to_scancode(const char* name) {
    for (const KeyNameEntry* e = KEY_NAME_TABLE; e->name; e++) {
        if (strcmp(name, e->name) == 0) {
            return e->code;
        }
    }
    return SDL_SCANCODE_UNKNOWN;
}

static KeyAction action_name_to_key_action(const char* name) {
    for (const ActionNameEntry* e = ACTION_NAME_TABLE; e->name; e++) {
        if (strcmp(name, e->name) == 0) {
            return e->action;
        }
    }
    return KEY_ACTION_COUNT; // invalid
}

static void trim(char* s) {
    // Trim leading whitespace
    char* start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);
    // Trim trailing whitespace
    char* end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) *end-- = '\0';
}

void keymap_load_defaults(Keymap* km) {
    km->bindings[KEY_P1_MOVE_UP]    = SDL_SCANCODE_W;
    km->bindings[KEY_P1_MOVE_DOWN]  = SDL_SCANCODE_S;
    km->bindings[KEY_P1_MOVE_LEFT]  = SDL_SCANCODE_A;
    km->bindings[KEY_P1_MOVE_RIGHT] = SDL_SCANCODE_D;
    km->bindings[KEY_P1_SHOOT]      = SDL_SCANCODE_SPACE;
    km->bindings[KEY_P2_MOVE_UP]    = SDL_SCANCODE_UP;
    km->bindings[KEY_P2_MOVE_DOWN]  = SDL_SCANCODE_DOWN;
    km->bindings[KEY_P2_MOVE_LEFT]  = SDL_SCANCODE_LEFT;
    km->bindings[KEY_P2_MOVE_RIGHT] = SDL_SCANCODE_RIGHT;
    km->bindings[KEY_P2_SHOOT]      = SDL_SCANCODE_RSHIFT;
    km->bindings[KEY_SCREENSHOT]    = SDL_SCANCODE_T;
    km->bindings[KEY_PAUSE]         = SDL_SCANCODE_P;
    km->bindings[KEY_QUIT]          = SDL_SCANCODE_ESCAPE;
}

int keymap_load(Keymap* km, const char* path) {
    // Start with defaults
    keymap_load_defaults(km);

    FILE* f = fopen(path, "r");
    if (!f) {
        return -1;
    }

    char line[256];
    int line_num = 0;
    while (fgets(line, sizeof(line), f)) {
        line_num++;
        trim(line);

        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#') continue;

        // Split on '='
        char* eq = strchr(line, '=');
        if (!eq) {
            fprintf(stderr, "keymap.cfg:%d: missing '='\n", line_num);
            continue;
        }

        *eq = '\0';
        char* key = line;
        char* value = eq + 1;
        trim(key);
        trim(value);

        KeyAction action = action_name_to_key_action(key);
        if (action == KEY_ACTION_COUNT) {
            fprintf(stderr, "keymap.cfg:%d: unknown action '%s'\n", line_num, key);
            continue;
        }

        SDL_Scancode code = key_name_to_scancode(value);
        if (code == SDL_SCANCODE_UNKNOWN) {
            fprintf(stderr, "keymap.cfg:%d: unknown key '%s'\n", line_num, value);
            continue;
        }

        km->bindings[action] = code;
    }

    fclose(f);
    return 0;
}

void keymap_get_actions(const Keymap* km, const Uint8* kb_state,
                        const GameState* state, PlayerAction out[2]) {
    out[0].move = ACTION_NOOP;
    out[0].shoot = ACTION_NOOP;
    out[1].move = ACTION_NOOP;
    out[1].shoot = ACTION_NOOP;

    // Player 1 movement (priority: last checked wins, but typically only one held)
    if (kb_state[km->bindings[KEY_P1_MOVE_UP]])    out[0].move = ACTION_UP;
    if (kb_state[km->bindings[KEY_P1_MOVE_DOWN]])  out[0].move = ACTION_DOWN;
    if (kb_state[km->bindings[KEY_P1_MOVE_LEFT]])  out[0].move = ACTION_LEFT;
    if (kb_state[km->bindings[KEY_P1_MOVE_RIGHT]]) out[0].move = ACTION_RIGHT;

    // Player 1 shoot — uses current facing direction
    if (kb_state[km->bindings[KEY_P1_SHOOT]]) {
        switch (state->players[0].facing) {
            case DIR_UP:    out[0].shoot = ACTION_UP;    break;
            case DIR_DOWN:  out[0].shoot = ACTION_DOWN;  break;
            case DIR_LEFT:  out[0].shoot = ACTION_LEFT;  break;
            case DIR_RIGHT: out[0].shoot = ACTION_RIGHT; break;
            default:        out[0].shoot = ACTION_UP;    break;
        }
    }

    // Player 2 movement
    if (kb_state[km->bindings[KEY_P2_MOVE_UP]])    out[1].move = ACTION_UP;
    if (kb_state[km->bindings[KEY_P2_MOVE_DOWN]])  out[1].move = ACTION_DOWN;
    if (kb_state[km->bindings[KEY_P2_MOVE_LEFT]])  out[1].move = ACTION_LEFT;
    if (kb_state[km->bindings[KEY_P2_MOVE_RIGHT]]) out[1].move = ACTION_RIGHT;

    // Player 2 shoot — uses current facing direction
    if (kb_state[km->bindings[KEY_P2_SHOOT]]) {
        switch (state->players[1].facing) {
            case DIR_UP:    out[1].shoot = ACTION_UP;    break;
            case DIR_DOWN:  out[1].shoot = ACTION_DOWN;  break;
            case DIR_LEFT:  out[1].shoot = ACTION_LEFT;  break;
            case DIR_RIGHT: out[1].shoot = ACTION_RIGHT; break;
            default:        out[1].shoot = ACTION_UP;    break;
        }
    }
}
