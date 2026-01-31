#ifndef ARENA_GAME_H
#define ARENA_GAME_H

#include "types.h"

// Initialize game state with an arena
void game_init(GameState* state, const char* map_str);

// Reset the game to initial state (keeps same arena)
void game_reset(GameState* state);

// Execute one game step with player actions
// Resolution order:
//   1. Entity collection (crystals)
//   2. Shooting (both players simultaneously)
//   3. Pushback (from hits)
//   4. Movement (both players simultaneously)
// Returns step info for reward calculation
StepInfo game_step(GameState* state, const PlayerAction actions[MAX_PLAYERS]);

// Check win conditions and update game_over/winner
void game_check_win_conditions(GameState* state);

// Find a valid respawn position for a player
Position game_find_respawn_position(const GameState* state, int player_idx);

// Tick all timers (cooldowns, crystals, etc.)
void game_tick_timers(GameState* state);

// Internal step phases (exposed for testing)
void game_phase_collect_crystals(GameState* state, StepInfo* info);
void game_phase_shooting(GameState* state, const PlayerAction actions[MAX_PLAYERS], StepInfo* info);
void game_phase_movement(GameState* state, const PlayerAction actions[MAX_PLAYERS], StepInfo* info);

#endif // ARENA_GAME_H
