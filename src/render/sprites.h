#ifndef ARENA_SPRITES_H
#define ARENA_SPRITES_H

#include <SDL.h>
#include <stdbool.h>
#include "../core/types.h"

// Sprite sheet configuration
#define SPRITE_SHEET_COLS 5
#define SPRITE_SHEET_ROWS 4
#define SPRITE_SIZE 32

// Sprite indices (matches sprite sheet layout)
typedef enum {
    // Row 0: Tiles
    SPRITE_TILE_FLOOR = 0,
    SPRITE_TILE_WALL = 1,
    SPRITE_TILE_VOID = 2,

    // Row 1: Crystals
    SPRITE_CRYSTAL_AVAILABLE = 5,
    SPRITE_CRYSTAL_COOLDOWN = 6,

    // Row 2: Player 1
    SPRITE_PLAYER1_UP = 10,
    SPRITE_PLAYER1_DOWN = 11,
    SPRITE_PLAYER1_LEFT = 12,
    SPRITE_PLAYER1_RIGHT = 13,
    SPRITE_PLAYER1_DEAD = 14,

    // Row 3: Player 2
    SPRITE_PLAYER2_UP = 15,
    SPRITE_PLAYER2_DOWN = 16,
    SPRITE_PLAYER2_LEFT = 17,
    SPRITE_PLAYER2_RIGHT = 18,
    SPRITE_PLAYER2_DEAD = 19,

    SPRITE_COUNT = 20
} SpriteIndex;

// Sprite sheet structure
typedef struct {
    SDL_Texture* texture;
    SDL_Rect regions[SPRITE_COUNT];
    bool loaded;
} SpriteSheet;

// Load sprite sheet from PNG file
// Returns 0 on success, -1 on failure
int sprites_load(SpriteSheet* sheet, SDL_Renderer* renderer, const char* path);

// Cleanup sprite sheet resources
void sprites_cleanup(SpriteSheet* sheet);

// Render a sprite at screen position (x, y)
void sprites_render(SDL_Renderer* renderer, const SpriteSheet* sheet,
                    SpriteIndex sprite, int x, int y);

// Render a sprite to a destination rectangle (for scaling)
void sprites_render_rect(SDL_Renderer* renderer, const SpriteSheet* sheet,
                         SpriteIndex sprite, const SDL_Rect* dest);

// Mapping functions: convert game types to sprite indices
SpriteIndex sprite_for_tile(TileType tile);
SpriteIndex sprite_for_crystal(bool on_cooldown);
SpriteIndex sprite_for_player(int player_index, Direction facing, bool alive);

#endif // ARENA_SPRITES_H
