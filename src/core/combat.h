#ifndef ARENA_COMBAT_H
#define ARENA_COMBAT_H

#include "types.h"

// Fire a laser from shooter in direction
// Returns information about the shot result
// Does NOT modify game state - use combat_apply_laser_result for that
LaserResult combat_fire_laser(
    const GameState* state,
    int shooter_idx,
    Direction dir
);

// Apply the result of a laser hit to game state
// Handles damage and pushback
void combat_apply_laser_result(
    GameState* state,
    int shooter_idx,
    const LaserResult* result
);

// Apply pushback to a player
// Returns the final position after pushback (may be same if blocked by wall)
// Sets fragged to true if player was pushed into void
Position combat_apply_pushback(
    const GameState* state,
    int player_idx,
    Direction push_dir,
    int distance,
    bool* fragged
);

// Check line of sight between two positions
// Returns true if there's a clear path (no walls)
bool combat_has_line_of_sight(
    const Arena* arena,
    Position from,
    Position to
);

#endif // ARENA_COMBAT_H
