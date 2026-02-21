#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include "../core/types.h"
#include "../core/game.h"
#include "render.h"
#include "screenshot.h"
#include "keymap.h"

// Tick rate in milliseconds (60 ticks per second)
#define TICK_MS 16

// Simple test map
static const char* TEST_MAP =
    "########\n"
    "#......#\n"
    "#..*..*#\n"
    "#.1..2.#\n"
    "#..*..*#\n"
    "#......#\n"
    "########\n";

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    // Initialize game state
    GameState state;
    game_init(&state, TEST_MAP);

    printf("Arena: %dx%d\n", state.arena.width, state.arena.height);
    printf("Players: P1 at (%d,%d), P2 at (%d,%d)\n",
           state.players[0].pos.x, state.players[0].pos.y,
           state.players[1].pos.x, state.players[1].pos.y);
    printf("Crystals: %d\n", state.arena.num_crystals);

    // Initialize renderer
    RenderContext ctx;
    if (render_init(&ctx, state.arena.width, state.arena.height) < 0) {
        fprintf(stderr, "Failed to initialize renderer\n");
        return 1;
    }

    // Load keymap
    Keymap keymap;
    if (keymap_load(&keymap, "keymap.cfg") < 0) {
        printf("No keymap.cfg found, using defaults\n");
        keymap_load_defaults(&keymap);
    } else {
        printf("Loaded keymap from keymap.cfg\n");
    }

    // Enable alpha blending for laser fade
    SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);

    // Main loop
    bool running = true;
    bool paused = false;
    SDL_Event event;
    Uint32 last_tick_time = SDL_GetTicks();

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && !event.key.repeat) {
                SDL_Scancode sc = event.key.keysym.scancode;
                if (sc == keymap.bindings[KEY_QUIT]) {
                    running = false;
                }
                if (sc == keymap.bindings[KEY_PAUSE]) {
                    paused = !paused;
                    if (paused) {
                        printf("Paused\n");
                    } else {
                        printf("Resumed\n");
                        last_tick_time = SDL_GetTicks();
                    }
                }
                if (sc == keymap.bindings[KEY_SCREENSHOT]) {
                    screenshot_save(&ctx);
                }
            }
        }

        if (!paused && !state.game_over) {
            Uint32 now = SDL_GetTicks();
            while (now - last_tick_time >= TICK_MS) {
                const Uint8* kb_state = SDL_GetKeyboardState(NULL);
                PlayerAction actions[2];
                keymap_get_actions(&keymap, kb_state, &state, actions);
                game_step(&state, actions);
                last_tick_time += TICK_MS;
            }
        }

        render_game(&ctx, &state);
    }

    render_cleanup(&ctx);
    printf("Goodbye!\n");

    return 0;
}
