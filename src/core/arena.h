#ifndef ARENA_ARENA_H
#define ARENA_ARENA_H

#include "types.h"

// Initialize an empty arena
void arena_init(Arena* arena, int width, int height);

// Load arena from string (ASCII art format)
// Returns true on success, false on error
bool arena_load_from_string(Arena* arena, const char* map_str);

// Tile queries
TileType arena_get_tile(const Arena* arena, int x, int y);
bool arena_is_passable(const Arena* arena, int x, int y);
bool arena_is_valid_position(const Arena* arena, int x, int y);
bool arena_is_void(const Arena* arena, int x, int y);
bool arena_is_wall(const Arena* arena, int x, int y);

// Crystal queries
int arena_get_crystal_at(const Arena* arena, int x, int y);  // returns crystal index or -1
bool arena_crystal_available(const Arena* arena, int crystal_idx);

// Crystal mutations
void arena_collect_crystal(Arena* arena, int crystal_idx);
void arena_tick_crystals(Arena* arena);  // decrement cooldowns

// Position helpers
Position position_add_direction(Position pos, Direction dir);
Direction action_to_direction(ActionType action);
int manhattan_distance(Position a, Position b);

#endif // ARENA_ARENA_H
