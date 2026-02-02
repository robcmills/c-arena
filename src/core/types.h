#ifndef ARENA_TYPES_H
#define ARENA_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// Constants from specification
// =============================================================================

// Timing (in milliseconds)
#define TICK_RATE_MS        16.6667f  // 60 ticks per second
#define EPISODE_LENGTH_MS   120000    // 120 seconds
#define MOVEMENT_COOLDOWN_MS 200
#define LASER_COOLDOWN_MS   200
#define CRYSTAL_RESPAWN_MS  8000
#define ENERGY_REGEN_MS     2000      // 1 energy per 2 seconds

// Timing (in ticks at 60fps)
#define MOVEMENT_COOLDOWN_TICKS 12    // 200ms / 16.67ms
#define LASER_COOLDOWN_TICKS    12    // 200ms / 16.67ms
#define CRYSTAL_RESPAWN_TICKS   480   // 8000ms / 16.67ms
#define ENERGY_REGEN_TICKS      120   // 2000ms / 16.67ms
#define EPISODE_LENGTH_TICKS    7200  // 120000ms / 16.67ms

// Player stats
#define MAX_HEALTH          4
#define MAX_ENERGY          8
#define STARTING_HEALTH     4
#define STARTING_ENERGY     8

// Combat
#define LASER_DAMAGE        1
#define PUSHBACK_DISTANCE   1

// Win condition
#define WIN_SCORE           8

// Arena limits
#define MAX_ARENA_WIDTH     32
#define MAX_ARENA_HEIGHT    32
#define MAX_PLAYERS         2
#define MAX_CRYSTALS        8
#define MAX_SPAWN_POINTS    4

// Respawn
#define RESPAWN_MIN_DISTANCE 3  // Manhattan distance from opponent

// =============================================================================
// Enums
// =============================================================================

typedef enum {
    TILE_FLOOR = 0,
    TILE_WALL  = 1,
    TILE_VOID  = 2
} TileType;

typedef enum {
    DIR_NONE  = 0,
    DIR_UP    = 1,
    DIR_DOWN  = 2,
    DIR_LEFT  = 3,
    DIR_RIGHT = 4
} Direction;

typedef enum {
    ACTION_NOOP  = 0,
    ACTION_UP    = 1,
    ACTION_DOWN  = 2,
    ACTION_LEFT  = 3,
    ACTION_RIGHT = 4
} ActionType;

typedef enum {
    LASER_HIT_NONE   = 0,  // No shot fired (DIR_NONE)
    LASER_HIT_PLAYER = 1,  // Hit another player
    LASER_HIT_WALL   = 2,  // Hit a wall
    LASER_HIT_EDGE   = 3   // Hit the arena edge (out of bounds)
} LaserHitType;

// =============================================================================
// Structures
// =============================================================================

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    Position pos;
    int cooldown_ticks;  // 0 = available, >0 = on cooldown
} Crystal;

typedef struct {
    Position pos;
} SpawnPoint;

typedef struct {
    int width;
    int height;
    TileType tiles[MAX_ARENA_HEIGHT][MAX_ARENA_WIDTH];

    int num_crystals;
    Crystal crystals[MAX_CRYSTALS];

    int num_spawn_points;
    SpawnPoint spawn_points[MAX_SPAWN_POINTS];
} Arena;

typedef struct {
    Position pos;
    int health;
    int energy;
    int move_cooldown_ticks;   // 0 = can move
    int laser_cooldown_ticks;  // 0 = can shoot
    int energy_regen_ticks;    // countdown to next energy regen
    int score;
    bool alive;
} Player;

// Action for a single player: move direction + shoot direction
typedef struct {
    ActionType move;
    ActionType shoot;
} PlayerAction;

// Full game state
typedef struct {
    Arena arena;
    Player players[MAX_PLAYERS];
    int current_tick;
    int winner;  // -1 = no winner yet, 0 or 1 = player index who won
    bool game_over;
} GameState;

// Result of a laser shot (for debugging/rendering)
typedef struct {
    LaserHitType hit_type;  // What the laser hit
    int target_player;      // -1 if no player hit
    Position hit_position;  // where the laser stopped
    Position pushback_to;   // where target was pushed to
    bool target_fragged;    // true if pushback caused frag
} LaserResult;

// Step result info (for Python bindings)
typedef struct {
    bool player_hit[MAX_PLAYERS];
    bool player_fragged[MAX_PLAYERS];
    bool crystal_collected[MAX_PLAYERS];
    int damage_dealt[MAX_PLAYERS];
    int damage_taken[MAX_PLAYERS];
} StepInfo;

#endif // ARENA_TYPES_H
