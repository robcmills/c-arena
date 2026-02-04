#ifndef ARENA_RENDER_H
#define ARENA_RENDER_H

#include <SDL.h>
#include "../core/types.h"

// Rendering constants
#define TILE_SIZE 32
#define HUD_HEIGHT 60

// Colors (RGBA)
typedef struct {
    Uint8 r, g, b, a;
} Color;

// Renderer context
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int window_width;
    int window_height;
} RenderContext;

// Initialize SDL and create window/renderer
// Returns 0 on success, -1 on failure
int render_init(RenderContext* ctx, int arena_width, int arena_height);

// Cleanup SDL resources
void render_cleanup(RenderContext* ctx);

// Render the full game state
void render_game(RenderContext* ctx, const GameState* state);

// Individual render functions (for flexibility)
void render_arena(RenderContext* ctx, const Arena* arena);
void render_crystals(RenderContext* ctx, const Arena* arena);
void render_players(RenderContext* ctx, const Player players[MAX_PLAYERS]);
void render_hud(RenderContext* ctx, const GameState* state);

#endif // ARENA_RENDER_H
