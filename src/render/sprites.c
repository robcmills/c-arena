#include "sprites.h"
#include <SDL_image.h>
#include <stdio.h>

int sprites_load(SpriteSheet* sheet, SDL_Renderer* renderer, const char* path) {
    sheet->loaded = false;
    sheet->texture = NULL;

    // Load the PNG file as a surface
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        fprintf(stderr, "Failed to load sprite sheet '%s': %s\n", path, IMG_GetError());
        return -1;
    }

    // Create texture from surface
    sheet->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!sheet->texture) {
        fprintf(stderr, "Failed to create sprite texture: %s\n", SDL_GetError());
        return -1;
    }

    // Pre-compute all sprite regions based on grid layout
    for (int i = 0; i < SPRITE_COUNT; i++) {
        int row = i / SPRITE_SHEET_COLS;
        int col = i % SPRITE_SHEET_COLS;
        sheet->regions[i] = (SDL_Rect){
            col * SPRITE_SIZE,
            row * SPRITE_SIZE,
            SPRITE_SIZE,
            SPRITE_SIZE
        };
    }

    sheet->loaded = true;
    printf("Loaded sprite sheet: %s\n", path);
    return 0;
}

void sprites_cleanup(SpriteSheet* sheet) {
    if (sheet->texture) {
        SDL_DestroyTexture(sheet->texture);
        sheet->texture = NULL;
    }
    sheet->loaded = false;
}

void sprites_render(SDL_Renderer* renderer, const SpriteSheet* sheet,
                    SpriteIndex sprite, int x, int y) {
    if (!sheet->loaded || sprite >= SPRITE_COUNT) {
        return;
    }

    SDL_Rect dest = {x, y, SPRITE_SIZE, SPRITE_SIZE};
    SDL_RenderCopy(renderer, sheet->texture, &sheet->regions[sprite], &dest);
}

void sprites_render_rect(SDL_Renderer* renderer, const SpriteSheet* sheet,
                         SpriteIndex sprite, const SDL_Rect* dest) {
    if (!sheet->loaded || sprite >= SPRITE_COUNT) {
        return;
    }

    SDL_RenderCopy(renderer, sheet->texture, &sheet->regions[sprite], dest);
}

SpriteIndex sprite_for_tile(TileType tile) {
    switch (tile) {
        case TILE_FLOOR: return SPRITE_TILE_FLOOR;
        case TILE_WALL:  return SPRITE_TILE_WALL;
        case TILE_VOID:  return SPRITE_TILE_VOID;
        default:         return SPRITE_TILE_VOID;
    }
}

SpriteIndex sprite_for_crystal(bool on_cooldown) {
    return on_cooldown ? SPRITE_CRYSTAL_COOLDOWN : SPRITE_CRYSTAL_AVAILABLE;
}

SpriteIndex sprite_for_player(int player_index, Direction facing, bool alive) {
    if (!alive) {
        return (player_index == 0) ? SPRITE_PLAYER1_DEAD : SPRITE_PLAYER2_DEAD;
    }

    int base = (player_index == 0) ? SPRITE_PLAYER1_UP : SPRITE_PLAYER2_UP;

    switch (facing) {
        case DIR_UP:    return base + 0;
        case DIR_DOWN:  return base + 1;
        case DIR_LEFT:  return base + 2;
        case DIR_RIGHT: return base + 3;
        default:        return base + 1;  // Default to DOWN
    }
}
