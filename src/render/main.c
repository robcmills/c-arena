#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include "../core/types.h"
#include "../core/game.h"
#include "render.h"

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

    // Main loop
    bool running = true;
    SDL_Event event;

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
                if (event.key.keysym.sym == SDLK_q) {
                    running = false;
                }
            }
        }

        // Render
        render_game(&ctx, &state);
    }

    // Cleanup
    render_cleanup(&ctx);
    printf("Goodbye!\n");

    return 0;
}
