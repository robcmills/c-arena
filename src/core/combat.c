#include "combat.h"
#include "arena.h"
#include "player.h"

// Get direction vector
static void get_direction_delta(Direction dir, int* dx, int* dy) {
    *dx = 0;
    *dy = 0;
    switch (dir) {
        case DIR_UP:    *dy = -1; break;
        case DIR_DOWN:  *dy = 1;  break;
        case DIR_LEFT:  *dx = -1; break;
        case DIR_RIGHT: *dx = 1;  break;
        default: break;
    }
}

// Get opposite direction for pushback
static Direction get_opposite_direction(Direction dir) {
    switch (dir) {
        case DIR_UP:    return DIR_DOWN;
        case DIR_DOWN:  return DIR_UP;
        case DIR_LEFT:  return DIR_RIGHT;
        case DIR_RIGHT: return DIR_LEFT;
        default:        return DIR_NONE;
    }
}

LaserResult combat_fire_laser(
    const GameState* state,
    int shooter_idx,
    Direction dir
) {
    LaserResult result = {
        .hit_type = LASER_HIT_NONE,
        .target_player = -1,
        .hit_position = state->players[shooter_idx].pos,
        .pushback_to = {-1, -1},
        .target_fragged = false
    };

    if (dir == DIR_NONE) {
        return result;
    }

    Position current = state->players[shooter_idx].pos;
    int dx, dy;
    get_direction_delta(dir, &dx, &dy);

    // Trace the laser path
    while (true) {
        current.x += dx;
        current.y += dy;

        // Check bounds
        if (!arena_is_valid_position(&state->arena, current.x, current.y)) {
            result.hit_type = LASER_HIT_EDGE;
            result.hit_position = current;
            break;
        }

        // Check for wall
        if (arena_is_wall(&state->arena, current.x, current.y)) {
            result.hit_type = LASER_HIT_WALL;
            result.hit_position = current;
            break;
        }

        // Check for player hit
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (i != shooter_idx &&
                state->players[i].alive &&
                state->players[i].pos.x == current.x &&
                state->players[i].pos.y == current.y) {
                result.hit_type = LASER_HIT_PLAYER;
                result.target_player = i;
                result.hit_position = current;

                // Calculate pushback position
                Direction push_dir = dir;  // Same direction as laser
                bool fragged = false;
                result.pushback_to = combat_apply_pushback(
                    state, i, push_dir, PUSHBACK_DISTANCE, &fragged
                );
                result.target_fragged = fragged;

                return result;
            }
        }

        // Void tiles don't stop the laser, but we need a termination condition
        // The laser continues through void until it hits something or goes out of bounds
    }

    return result;
}

void combat_apply_laser_result(
    GameState* state,
    int shooter_idx,
    const LaserResult* result
) {
    (void)shooter_idx;  // Unused, but kept for potential future use

    if (result->hit_type != LASER_HIT_PLAYER || result->target_player < 0) {
        return;
    }

    Player* target = &state->players[result->target_player];

    // Apply damage
    player_take_damage(target, LASER_DAMAGE);

    // Apply pushback position (even if they died, for consistency)
    if (!result->target_fragged) {
        target->pos = result->pushback_to;
    }
    // If fragged by pushback into void, the player is marked as not alive
    // and will be respawned in the main game loop
}

Position combat_apply_pushback(
    const GameState* state,
    int player_idx,
    Direction push_dir,
    int distance,
    bool* fragged
) {
    *fragged = false;

    Position current = state->players[player_idx].pos;
    int dx, dy;
    get_direction_delta(push_dir, &dx, &dy);

    for (int i = 0; i < distance; i++) {
        Position next = {current.x + dx, current.y + dy};

        // Check if next position is void (instant death)
        if (arena_is_void(&state->arena, next.x, next.y)) {
            *fragged = true;
            return next;  // Return void position for visualization
        }

        // Check if next position is wall (stop at current)
        if (arena_is_wall(&state->arena, next.x, next.y)) {
            return current;
        }

        // Check if next position is occupied by another player
        for (int j = 0; j < MAX_PLAYERS; j++) {
            if (j != player_idx &&
                state->players[j].alive &&
                state->players[j].pos.x == next.x &&
                state->players[j].pos.y == next.y) {
                // Blocked by another player, stop at current
                return current;
            }
        }

        // Valid move, update current
        current = next;
    }

    return current;
}

bool combat_has_line_of_sight(
    const Arena* arena,
    Position from,
    Position to
) {
    // Must be on same row or column
    if (from.x != to.x && from.y != to.y) {
        return false;
    }

    int dx = 0, dy = 0;

    if (to.x > from.x) dx = 1;
    else if (to.x < from.x) dx = -1;

    if (to.y > from.y) dy = 1;
    else if (to.y < from.y) dy = -1;

    Position current = from;
    while (current.x != to.x || current.y != to.y) {
        current.x += dx;
        current.y += dy;

        // Don't check the target position itself
        if (current.x == to.x && current.y == to.y) {
            break;
        }

        // Wall blocks line of sight
        if (arena_is_wall(arena, current.x, current.y)) {
            return false;
        }
    }

    return true;
}
