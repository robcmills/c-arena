#ifndef ARENA_API_H
#define ARENA_API_H

#include "types.h"

// =============================================================================
// External API for Python bindings
// These functions provide a clean interface for the training environment
// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif

// Game lifecycle
void api_game_init(GameState* state, const char* map_str);
void api_game_reset(GameState* state);
void api_game_set_seed(unsigned int seed);

// Main step function
// actions: array of 2 integers per player [move, shoot] for each player
// So for 2 players: [p0_move, p0_shoot, p1_move, p1_shoot]
StepInfo api_game_step(GameState* state, const int* actions);

// State queries for observations
int api_get_arena_width(const GameState* state);
int api_get_arena_height(const GameState* state);
TileType api_get_tile(const GameState* state, int x, int y);

// Crystal queries
int api_get_num_crystals(const GameState* state);
int api_get_crystal_x(const GameState* state, int idx);
int api_get_crystal_y(const GameState* state, int idx);
int api_get_crystal_cooldown(const GameState* state, int idx);
bool api_is_crystal_available(const GameState* state, int idx);

// Player queries
int api_get_player_x(const GameState* state, int player_idx);
int api_get_player_y(const GameState* state, int player_idx);
int api_get_player_health(const GameState* state, int player_idx);
int api_get_player_energy(const GameState* state, int player_idx);
int api_get_player_move_cooldown(const GameState* state, int player_idx);
int api_get_player_laser_cooldown(const GameState* state, int player_idx);
int api_get_player_score(const GameState* state, int player_idx);
bool api_is_player_alive(const GameState* state, int player_idx);

// Game state queries
int api_get_current_tick(const GameState* state);
int api_get_winner(const GameState* state);
bool api_is_game_over(const GameState* state);

// Size query for allocation
int api_get_state_size(void);

#ifdef __cplusplus
}
#endif

#endif // ARENA_API_H
