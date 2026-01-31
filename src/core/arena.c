#include "arena.h"
#include <string.h>
#include <stdlib.h>

void arena_init(Arena* arena, int width, int height) {
    arena->width = width;
    arena->height = height;
    arena->num_crystals = 0;
    arena->num_spawn_points = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            arena->tiles[y][x] = TILE_FLOOR;
        }
    }

    for (int i = 0; i < MAX_CRYSTALS; i++) {
        arena->crystals[i].pos.x = -1;
        arena->crystals[i].pos.y = -1;
        arena->crystals[i].cooldown_ticks = 0;
    }

    for (int i = 0; i < MAX_SPAWN_POINTS; i++) {
        arena->spawn_points[i].pos.x = -1;
        arena->spawn_points[i].pos.y = -1;
    }
}

bool arena_load_from_string(Arena* arena, const char* map_str) {
    // First pass: determine dimensions
    int width = 0;
    int height = 0;
    int current_width = 0;

    for (const char* p = map_str; *p; p++) {
        if (*p == '\n') {
            if (current_width > width) {
                width = current_width;
            }
            if (current_width > 0) {
                height++;
            }
            current_width = 0;
        } else if (*p != ' ') {
            current_width++;
        }
    }
    // Handle last line without newline
    if (current_width > 0) {
        if (current_width > width) {
            width = current_width;
        }
        height++;
    }

    if (width > MAX_ARENA_WIDTH || height > MAX_ARENA_HEIGHT) {
        return false;
    }

    arena_init(arena, width, height);

    // Second pass: parse tiles
    int x = 0;
    int y = 0;

    for (const char* p = map_str; *p; p++) {
        if (*p == '\n') {
            x = 0;
            if (y < height - 1) {
                y++;
            }
            continue;
        }

        if (*p == ' ') {
            continue;  // Skip spaces (used for formatting)
        }

        if (x >= width || y >= height) {
            continue;
        }

        // Parse character
        // × = void, ■ = wall, □ = floor, ◆ = crystal, ▷◁△▽ = spawn points
        // Also support ASCII: x = void, # = wall, . = floor, * = crystal, 1/2 = spawn

        // Check for multi-byte UTF-8 characters
        unsigned char c = (unsigned char)*p;

        if (c == 'x' || c == 'X') {
            arena->tiles[y][x] = TILE_VOID;
        } else if (c == '#') {
            arena->tiles[y][x] = TILE_WALL;
        } else if (c == '.' || c == '_') {
            arena->tiles[y][x] = TILE_FLOOR;
        } else if (c == '*' || c == 'C' || c == 'c') {
            arena->tiles[y][x] = TILE_FLOOR;
            if (arena->num_crystals < MAX_CRYSTALS) {
                arena->crystals[arena->num_crystals].pos.x = x;
                arena->crystals[arena->num_crystals].pos.y = y;
                arena->crystals[arena->num_crystals].cooldown_ticks = 0;
                arena->num_crystals++;
            }
        } else if (c == '1' || c == '2' || c == 'S' || c == 's') {
            arena->tiles[y][x] = TILE_FLOOR;
            if (arena->num_spawn_points < MAX_SPAWN_POINTS) {
                arena->spawn_points[arena->num_spawn_points].pos.x = x;
                arena->spawn_points[arena->num_spawn_points].pos.y = y;
                arena->num_spawn_points++;
            }
        } else if (c >= 0xC0) {
            // Multi-byte UTF-8 character
            // ×(U+00D7): C3 97
            // ■(U+25A0): E2 96 A0
            // □(U+25A1): E2 96 A1
            // ◆(U+25C6): E2 97 86
            // ▷(U+25B7): E2 96 B7
            // ◁(U+25C1): E2 97 81

            if (c == 0xC3 && (unsigned char)*(p+1) == 0x97) {
                // × (multiplication sign) = void
                arena->tiles[y][x] = TILE_VOID;
                p++;
            } else if (c == 0xE2) {
                unsigned char c2 = (unsigned char)*(p+1);
                unsigned char c3 = (unsigned char)*(p+2);

                if (c2 == 0x96 && c3 == 0xA0) {
                    // ■ = wall
                    arena->tiles[y][x] = TILE_WALL;
                    p += 2;
                } else if (c2 == 0x96 && c3 == 0xA1) {
                    // □ = floor
                    arena->tiles[y][x] = TILE_FLOOR;
                    p += 2;
                } else if (c2 == 0x97 && c3 == 0x86) {
                    // ◆ = crystal
                    arena->tiles[y][x] = TILE_FLOOR;
                    if (arena->num_crystals < MAX_CRYSTALS) {
                        arena->crystals[arena->num_crystals].pos.x = x;
                        arena->crystals[arena->num_crystals].pos.y = y;
                        arena->crystals[arena->num_crystals].cooldown_ticks = 0;
                        arena->num_crystals++;
                    }
                    p += 2;
                } else if ((c2 == 0x96 && c3 == 0xB7) ||  // ▷
                           (c2 == 0x97 && c3 == 0x81) ||  // ◁
                           (c2 == 0x96 && c3 == 0xB3) ||  // △
                           (c2 == 0x96 && c3 == 0xBD)) {  // ▽
                    // Spawn point
                    arena->tiles[y][x] = TILE_FLOOR;
                    if (arena->num_spawn_points < MAX_SPAWN_POINTS) {
                        arena->spawn_points[arena->num_spawn_points].pos.x = x;
                        arena->spawn_points[arena->num_spawn_points].pos.y = y;
                        arena->num_spawn_points++;
                    }
                    p += 2;
                } else {
                    // Unknown UTF-8, treat as floor
                    arena->tiles[y][x] = TILE_FLOOR;
                    p += 2;
                }
            } else {
                // Other multi-byte, skip appropriately
                if (c >= 0xE0) p += 2;
                else p++;
            }
        } else {
            // Unknown single-byte, treat as floor
            arena->tiles[y][x] = TILE_FLOOR;
        }

        x++;
    }

    return true;
}

TileType arena_get_tile(const Arena* arena, int x, int y) {
    if (x < 0 || x >= arena->width || y < 0 || y >= arena->height) {
        return TILE_VOID;  // Out of bounds is void
    }
    return arena->tiles[y][x];
}

bool arena_is_passable(const Arena* arena, int x, int y) {
    TileType tile = arena_get_tile(arena, x, y);
    return tile == TILE_FLOOR;
}

bool arena_is_valid_position(const Arena* arena, int x, int y) {
    return x >= 0 && x < arena->width && y >= 0 && y < arena->height;
}

bool arena_is_void(const Arena* arena, int x, int y) {
    return arena_get_tile(arena, x, y) == TILE_VOID;
}

bool arena_is_wall(const Arena* arena, int x, int y) {
    return arena_get_tile(arena, x, y) == TILE_WALL;
}

int arena_get_crystal_at(const Arena* arena, int x, int y) {
    for (int i = 0; i < arena->num_crystals; i++) {
        if (arena->crystals[i].pos.x == x && arena->crystals[i].pos.y == y) {
            return i;
        }
    }
    return -1;
}

bool arena_crystal_available(const Arena* arena, int crystal_idx) {
    if (crystal_idx < 0 || crystal_idx >= arena->num_crystals) {
        return false;
    }
    return arena->crystals[crystal_idx].cooldown_ticks == 0;
}

void arena_collect_crystal(Arena* arena, int crystal_idx) {
    if (crystal_idx >= 0 && crystal_idx < arena->num_crystals) {
        arena->crystals[crystal_idx].cooldown_ticks = CRYSTAL_RESPAWN_TICKS;
    }
}

void arena_tick_crystals(Arena* arena) {
    for (int i = 0; i < arena->num_crystals; i++) {
        if (arena->crystals[i].cooldown_ticks > 0) {
            arena->crystals[i].cooldown_ticks--;
        }
    }
}

Position position_add_direction(Position pos, Direction dir) {
    Position result = pos;
    switch (dir) {
        case DIR_UP:    result.y--; break;
        case DIR_DOWN:  result.y++; break;
        case DIR_LEFT:  result.x--; break;
        case DIR_RIGHT: result.x++; break;
        default: break;
    }
    return result;
}

Direction action_to_direction(ActionType action) {
    switch (action) {
        case ACTION_UP:    return DIR_UP;
        case ACTION_DOWN:  return DIR_DOWN;
        case ACTION_LEFT:  return DIR_LEFT;
        case ACTION_RIGHT: return DIR_RIGHT;
        default:           return DIR_NONE;
    }
}

int manhattan_distance(Position a, Position b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return dx + dy;
}
