#include "game.h"
#include "arena.h"
#include "player.h"
#include "combat.h"
#include <stdlib.h>
#include <time.h>

// Simple random number generator state (for reproducibility in training)
static unsigned int rng_state = 12345;

static unsigned int game_rand(void) {
    rng_state = rng_state * 1103515245 + 12345;
    return (rng_state >> 16) & 0x7FFF;
}

void game_set_seed(unsigned int seed) {
    rng_state = seed;
}

void game_init(GameState* state, const char* map_str) {
    // Load arena
    arena_load_from_string(&state->arena, map_str);

    // Initialize players at spawn points
    for (int i = 0; i < MAX_PLAYERS; i++) {
        Position spawn = {0, 0};
        if (i < state->arena.num_spawn_points) {
            spawn = state->arena.spawn_points[i].pos;
        }
        player_init(&state->players[i], spawn);
    }

    state->current_tick = 0;
    state->winner = -1;
    state->game_over = false;
}

void game_reset(GameState* state) {
    // Reset crystal cooldowns
    for (int i = 0; i < state->arena.num_crystals; i++) {
        state->arena.crystals[i].cooldown_ticks = 0;
    }

    // Reset players to spawn points
    for (int i = 0; i < MAX_PLAYERS; i++) {
        Position spawn = {0, 0};
        if (i < state->arena.num_spawn_points) {
            spawn = state->arena.spawn_points[i].pos;
        }
        player_init(&state->players[i], spawn);
    }

    state->current_tick = 0;
    state->winner = -1;
    state->game_over = false;
}

StepInfo game_step(GameState* state, const PlayerAction actions[MAX_PLAYERS]) {
    StepInfo info = {0};

    if (state->game_over) {
        return info;
    }

    // Resolution order per spec:
    // 1. Entity collection (crystals) - happens when player is on crystal tile
    // 2. Shooting (both players simultaneously)
    // 3. Pushback (applied as part of shooting)
    // 4. Movement (both players simultaneously)

    // Phase 1: Collect crystals (based on current positions before any moves)
    game_phase_collect_crystals(state, &info);

    // Phase 2 & 3: Shooting and pushback
    game_phase_shooting(state, actions, &info);

    // Handle respawns for players fragged by shooting
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players[i].alive) {
            int opponent = 1 - i;
            state->players[opponent].score++;
            info.player_fragged[i] = true;

            Position respawn = game_find_respawn_position(state, i);
            player_respawn(&state->players[i], respawn);
        }
    }

    // Phase 4: Movement
    game_phase_movement(state, actions, &info);

    // Handle respawns for players fragged by movement (pushed/moved into void)
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players[i].alive && !info.player_fragged[i]) {
            int opponent = 1 - i;
            state->players[opponent].score++;
            info.player_fragged[i] = true;

            Position respawn = game_find_respawn_position(state, i);
            player_respawn(&state->players[i], respawn);
        }
    }

    // Tick timers
    game_tick_timers(state);

    // Increment tick counter
    state->current_tick++;

    // Check win conditions
    game_check_win_conditions(state);

    return info;
}

void game_phase_collect_crystals(GameState* state, StepInfo* info) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players[i].alive) continue;

        int crystal_idx = arena_get_crystal_at(
            &state->arena,
            state->players[i].pos.x,
            state->players[i].pos.y
        );

        if (crystal_idx >= 0 && arena_crystal_available(&state->arena, crystal_idx)) {
            // Collect crystal - restore full energy
            player_restore_energy(&state->players[i], MAX_ENERGY);
            arena_collect_crystal(&state->arena, crystal_idx);
            info->crystal_collected[i] = true;
        }
    }
}

void game_phase_shooting(GameState* state, const PlayerAction actions[MAX_PLAYERS], StepInfo* info) {
    LaserResult results[MAX_PLAYERS];
    bool will_shoot[MAX_PLAYERS] = {false};

    // First, determine who will shoot and calculate results
    for (int i = 0; i < MAX_PLAYERS; i++) {
        Direction shoot_dir = action_to_direction(actions[i].shoot);

        if (shoot_dir != DIR_NONE &&
            player_can_shoot(&state->players[i])) {

            // Consume energy and start cooldown
            if (player_use_energy(&state->players[i], 1)) {
                player_start_laser_cooldown(&state->players[i]);
                will_shoot[i] = true;

                // Calculate where the shot would land
                results[i] = combat_fire_laser(state, i, shoot_dir);
            }
        }
    }

    // Apply all hits simultaneously
    // This means if both players shoot each other, both take damage
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (will_shoot[i] && results[i].hit) {
            int target = results[i].target_player;

            // Apply damage
            player_take_damage(&state->players[target], LASER_DAMAGE);
            info->player_hit[target] = true;
            info->damage_dealt[i] += LASER_DAMAGE;
            info->damage_taken[target] += LASER_DAMAGE;
        }
    }

    // Apply pushback after all damage (so simultaneous shots don't interfere)
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (will_shoot[i] && results[i].hit) {
            int target = results[i].target_player;

            // Only apply pushback if target is still alive
            // (they might have died from damage)
            if (state->players[target].alive) {
                if (results[i].target_fragged) {
                    // Pushed into void
                    state->players[target].alive = false;
                } else {
                    // Apply pushback position
                    state->players[target].pos = results[i].pushback_to;
                }
            }
        }
    }
}

void game_phase_movement(GameState* state, const PlayerAction actions[MAX_PLAYERS], StepInfo* info) {
    (void)info;  // Currently unused in movement phase

    Position intended[MAX_PLAYERS];
    bool wants_move[MAX_PLAYERS] = {false};

    // Calculate intended positions
    for (int i = 0; i < MAX_PLAYERS; i++) {
        intended[i] = state->players[i].pos;

        if (!state->players[i].alive) continue;

        Direction move_dir = action_to_direction(actions[i].move);

        if (move_dir != DIR_NONE && player_can_move(&state->players[i])) {
            Position target = position_add_direction(state->players[i].pos, move_dir);

            // Check if target is passable
            if (arena_is_passable(&state->arena, target.x, target.y)) {
                intended[i] = target;
                wants_move[i] = true;
            } else if (arena_is_void(&state->arena, target.x, target.y)) {
                // Moving into void = death
                intended[i] = target;
                wants_move[i] = true;
            }
            // Wall = stay in place, but still trigger cooldown if tried to move
        }
    }

    // Resolve conflicts
    // Check for swap attempts (A->B and B->A)
    bool swap_attempted = false;
    if (MAX_PLAYERS == 2) {
        if (wants_move[0] && wants_move[1]) {
            // Check if they're trying to swap positions
            if (intended[0].x == state->players[1].pos.x &&
                intended[0].y == state->players[1].pos.y &&
                intended[1].x == state->players[0].pos.x &&
                intended[1].y == state->players[0].pos.y) {
                // Swap blocked - both stay in place
                swap_attempted = true;
                wants_move[0] = false;
                wants_move[1] = false;
            }
        }
    }

    // Check for same-tile collision
    if (!swap_attempted && MAX_PLAYERS == 2) {
        if (wants_move[0] && wants_move[1]) {
            if (intended[0].x == intended[1].x &&
                intended[0].y == intended[1].y) {
                // Both trying to move to same tile - both cancelled
                wants_move[0] = false;
                wants_move[1] = false;
            }
        }
    }

    // Check for moving into opponent's current position (if opponent isn't moving)
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!wants_move[i]) continue;

        for (int j = 0; j < MAX_PLAYERS; j++) {
            if (i == j) continue;
            if (!state->players[j].alive) continue;

            // If moving to opponent's position and opponent isn't moving away
            if (intended[i].x == state->players[j].pos.x &&
                intended[i].y == state->players[j].pos.y) {
                // Check if opponent is moving away
                if (!wants_move[j] ||
                    (intended[j].x == state->players[j].pos.x &&
                     intended[j].y == state->players[j].pos.y)) {
                    // Opponent isn't moving or staying in place - block our move
                    wants_move[i] = false;
                }
            }
        }
    }

    // Apply movements
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (wants_move[i]) {
            // Check for void death
            if (arena_is_void(&state->arena, intended[i].x, intended[i].y)) {
                state->players[i].alive = false;
                // Position doesn't matter, they'll respawn
            } else {
                state->players[i].pos = intended[i];
            }
            player_start_move_cooldown(&state->players[i]);
        } else if (actions[i].move != ACTION_NOOP && state->players[i].alive) {
            // Tried to move but was blocked - still start cooldown
            Direction move_dir = action_to_direction(actions[i].move);
            if (move_dir != DIR_NONE && player_can_move(&state->players[i])) {
                player_start_move_cooldown(&state->players[i]);
            }
        }
    }

    // Collect crystals at new positions (after movement)
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players[i].alive) continue;

        int crystal_idx = arena_get_crystal_at(
            &state->arena,
            state->players[i].pos.x,
            state->players[i].pos.y
        );

        if (crystal_idx >= 0 && arena_crystal_available(&state->arena, crystal_idx)) {
            player_restore_energy(&state->players[i], MAX_ENERGY);
            arena_collect_crystal(&state->arena, crystal_idx);
            info->crystal_collected[i] = true;
        }
    }
}

void game_check_win_conditions(GameState* state) {
    // Check score win condition
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].score >= WIN_SCORE) {
            state->winner = i;
            state->game_over = true;
            return;
        }
    }

    // Check timeout
    if (state->current_tick >= EPISODE_LENGTH_TICKS) {
        state->game_over = true;
        // Determine winner by score
        if (state->players[0].score > state->players[1].score) {
            state->winner = 0;
        } else if (state->players[1].score > state->players[0].score) {
            state->winner = 1;
        } else {
            state->winner = -1;  // Draw
        }
    }
}

Position game_find_respawn_position(const GameState* state, int player_idx) {
    // Collect all valid floor tiles
    Position candidates[MAX_ARENA_WIDTH * MAX_ARENA_HEIGHT];
    int num_candidates = 0;

    int opponent_idx = 1 - player_idx;
    Position opponent_pos = state->players[opponent_idx].pos;

    for (int y = 0; y < state->arena.height; y++) {
        for (int x = 0; x < state->arena.width; x++) {
            if (arena_is_passable(&state->arena, x, y)) {
                Position pos = {x, y};

                // Check minimum distance from opponent
                if (manhattan_distance(pos, opponent_pos) >= RESPAWN_MIN_DISTANCE) {
                    // Check not occupied by other player
                    bool occupied = false;
                    for (int i = 0; i < MAX_PLAYERS; i++) {
                        if (i != player_idx &&
                            state->players[i].alive &&
                            state->players[i].pos.x == x &&
                            state->players[i].pos.y == y) {
                            occupied = true;
                            break;
                        }
                    }

                    if (!occupied) {
                        candidates[num_candidates++] = pos;
                    }
                }
            }
        }
    }

    // If no valid candidates (shouldn't happen with proper map design),
    // fall back to any floor tile
    if (num_candidates == 0) {
        for (int y = 0; y < state->arena.height; y++) {
            for (int x = 0; x < state->arena.width; x++) {
                if (arena_is_passable(&state->arena, x, y)) {
                    Position pos = {x, y};
                    bool occupied = false;
                    for (int i = 0; i < MAX_PLAYERS; i++) {
                        if (i != player_idx &&
                            state->players[i].alive &&
                            state->players[i].pos.x == x &&
                            state->players[i].pos.y == y) {
                            occupied = true;
                            break;
                        }
                    }
                    if (!occupied) {
                        candidates[num_candidates++] = pos;
                    }
                }
            }
        }
    }

    // Random selection
    if (num_candidates > 0) {
        int idx = game_rand() % num_candidates;
        return candidates[idx];
    }

    // Last resort - spawn at origin
    Position fallback = {0, 0};
    return fallback;
}

void game_tick_timers(GameState* state) {
    // Tick player cooldowns
    for (int i = 0; i < MAX_PLAYERS; i++) {
        player_tick_cooldowns(&state->players[i]);
    }

    // Tick crystal respawn timers
    arena_tick_crystals(&state->arena);
}
