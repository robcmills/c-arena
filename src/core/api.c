#include "api.h"
#include "game.h"
#include "arena.h"
#include "player.h"

// External declaration from game.c
extern void game_set_seed(unsigned int seed);

void api_game_init(GameState* state, const char* map_str) {
    game_init(state, map_str);
}

void api_game_reset(GameState* state) {
    game_reset(state);
}

void api_game_set_seed(unsigned int seed) {
    game_set_seed(seed);
}

StepInfo api_game_step(GameState* state, const int* actions) {
    PlayerAction player_actions[MAX_PLAYERS];

    for (int i = 0; i < MAX_PLAYERS; i++) {
        player_actions[i].move = (ActionType)actions[i * 2];
        player_actions[i].shoot = (ActionType)actions[i * 2 + 1];
    }

    return game_step(state, player_actions);
}

int api_get_arena_width(const GameState* state) {
    return state->arena.width;
}

int api_get_arena_height(const GameState* state) {
    return state->arena.height;
}

TileType api_get_tile(const GameState* state, int x, int y) {
    return arena_get_tile(&state->arena, x, y);
}

int api_get_num_crystals(const GameState* state) {
    return state->arena.num_crystals;
}

int api_get_crystal_x(const GameState* state, int idx) {
    if (idx < 0 || idx >= state->arena.num_crystals) return -1;
    return state->arena.crystals[idx].pos.x;
}

int api_get_crystal_y(const GameState* state, int idx) {
    if (idx < 0 || idx >= state->arena.num_crystals) return -1;
    return state->arena.crystals[idx].pos.y;
}

int api_get_crystal_cooldown(const GameState* state, int idx) {
    if (idx < 0 || idx >= state->arena.num_crystals) return -1;
    return state->arena.crystals[idx].cooldown_ticks;
}

bool api_is_crystal_available(const GameState* state, int idx) {
    return arena_crystal_available(&state->arena, idx);
}

int api_get_player_x(const GameState* state, int player_idx) {
    if (player_idx < 0 || player_idx >= MAX_PLAYERS) return -1;
    return state->players[player_idx].pos.x;
}

int api_get_player_y(const GameState* state, int player_idx) {
    if (player_idx < 0 || player_idx >= MAX_PLAYERS) return -1;
    return state->players[player_idx].pos.y;
}

int api_get_player_health(const GameState* state, int player_idx) {
    if (player_idx < 0 || player_idx >= MAX_PLAYERS) return -1;
    return state->players[player_idx].health;
}

int api_get_player_energy(const GameState* state, int player_idx) {
    if (player_idx < 0 || player_idx >= MAX_PLAYERS) return -1;
    return state->players[player_idx].energy;
}

int api_get_player_move_cooldown(const GameState* state, int player_idx) {
    if (player_idx < 0 || player_idx >= MAX_PLAYERS) return -1;
    return state->players[player_idx].move_cooldown_ticks;
}

int api_get_player_laser_cooldown(const GameState* state, int player_idx) {
    if (player_idx < 0 || player_idx >= MAX_PLAYERS) return -1;
    return state->players[player_idx].laser_cooldown_ticks;
}

int api_get_player_score(const GameState* state, int player_idx) {
    if (player_idx < 0 || player_idx >= MAX_PLAYERS) return -1;
    return state->players[player_idx].score;
}

bool api_is_player_alive(const GameState* state, int player_idx) {
    if (player_idx < 0 || player_idx >= MAX_PLAYERS) return false;
    return state->players[player_idx].alive;
}

int api_get_current_tick(const GameState* state) {
    return state->current_tick;
}

int api_get_winner(const GameState* state) {
    return state->winner;
}

bool api_is_game_over(const GameState* state) {
    return state->game_over;
}

int api_get_state_size(void) {
    return sizeof(GameState);
}
