#ifndef ARENA_KEYMAP_H
#define ARENA_KEYMAP_H

#include <SDL.h>
#include "../core/types.h"

typedef enum {
    KEY_P1_MOVE_UP = 0,
    KEY_P1_MOVE_DOWN,
    KEY_P1_MOVE_LEFT,
    KEY_P1_MOVE_RIGHT,
    KEY_P1_SHOOT,
    KEY_P2_MOVE_UP,
    KEY_P2_MOVE_DOWN,
    KEY_P2_MOVE_LEFT,
    KEY_P2_MOVE_RIGHT,
    KEY_P2_SHOOT,
    KEY_SCREENSHOT,
    KEY_PAUSE,
    KEY_QUIT,
    KEY_ACTION_COUNT
} KeyAction;

typedef struct {
    SDL_Scancode bindings[KEY_ACTION_COUNT];
} Keymap;

// Load keymap from config file. Returns 0 on success, -1 on failure.
int keymap_load(Keymap* km, const char* path);

// Load default keymap (fallback)
void keymap_load_defaults(Keymap* km);

// Poll keyboard state and build player actions for both players.
// Shoot uses player's current facing direction when shoot key is held.
void keymap_get_actions(const Keymap* km, const Uint8* kb_state,
                        const GameState* state, PlayerAction out[2]);

#endif // ARENA_KEYMAP_H
