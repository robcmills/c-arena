#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../src/core/types.h"
#include "../src/core/arena.h"
#include "../src/core/player.h"
#include "../src/core/combat.h"
#include "../src/core/game.h"
#include "../src/core/api.h"

// Simple test framework
static int tests_run = 0;
static int tests_passed = 0;
static int current_test_failed = 0;

// ANSI color codes
#define COLOR_GREEN "\033[32m"
#define COLOR_RED   "\033[31m"
#define COLOR_CYAN  "\033[36m"
#define COLOR_RESET "\033[0m"

#define TEST(name) void name(void)
#define RUN_TEST(name) do { \
    printf("  %s... ", #name); \
    tests_run++; \
    current_test_failed = 0; \
    name(); \
    if (!current_test_failed) { \
        tests_passed++; \
        printf(COLOR_GREEN "PASSED" COLOR_RESET "\n"); \
    } \
} while(0)

#define ASSERT(cond, msg) do { \
    if (!(cond)) { \
        printf(COLOR_RED "FAILED" COLOR_RESET "\n    Assertion failed: %s\n    at %s:%d\n    %s\n", \
               #cond, __FILE__, __LINE__, #msg); \
        current_test_failed = 1; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf(COLOR_RED "FAILED" COLOR_RESET "\n    Expected %d == %d\n    at %s:%d\n", \
               (int)(a), (int)(b), __FILE__, __LINE__); \
        current_test_failed = 1; \
        return; \
    } \
} while(0)

// Test maps
static const char* TEST_MAP_ASCII =
    "x # # # # # x\n"
    "x . . . . * x\n"
    "x 1 . . . . x\n"
    "x . . . . . x\n"
    "x . . . . 2 x\n"
    "x * . . . . x\n"
    "x # # # # # x\n";

static const char* TEST_MAP_UTF8 =
    "× ■ ■ ■ ■ ■ × \n"
    "× □ □ □ □ ◆ × \n"
    "× ▷ □ □ □ □ × \n"
    "× □ □ □ □ □ × \n"
    "× □ □ □ □ ◁ × \n"
    "× ◆ □ □ □ □ × \n"
    "× ■ ■ ■ ■ ■ × \n";

// =============================================================================
// Arena Tests
// =============================================================================

void test_arena_load(Arena* arena) {
    ASSERT_EQ(arena->width, 7);
    ASSERT_EQ(arena->height, 7);

    // Check corners are void
    ASSERT_EQ(arena_get_tile(arena, 0, 0), TILE_VOID);
    ASSERT_EQ(arena_get_tile(arena, 6, 0), TILE_VOID);
    ASSERT_EQ(arena_get_tile(arena, 0, 6), TILE_VOID);
    ASSERT_EQ(arena_get_tile(arena, 6, 6), TILE_VOID);

    // Check walls
    ASSERT_EQ(arena_get_tile(arena, 1, 0), TILE_WALL);
    ASSERT_EQ(arena_get_tile(arena, 5, 0), TILE_WALL);
    ASSERT_EQ(arena_get_tile(arena, 1, 6), TILE_WALL);
    ASSERT_EQ(arena_get_tile(arena, 5, 6), TILE_WALL);

    // Check floors
    ASSERT_EQ(arena_get_tile(arena, 1, 1), TILE_FLOOR);
    ASSERT_EQ(arena_get_tile(arena, 3, 3), TILE_FLOOR);

    // Check spawn points (should be floor)
    ASSERT_EQ(arena_get_tile(arena, 1, 2), TILE_FLOOR);
    ASSERT_EQ(arena_get_tile(arena, 5, 4), TILE_FLOOR);

    // Check crystal tiles (should be floor)
    ASSERT_EQ(arena_get_tile(arena, 5, 1), TILE_FLOOR);
    ASSERT_EQ(arena_get_tile(arena, 1, 5), TILE_FLOOR);

    // Check crystal positions
    ASSERT_EQ(arena->crystals[0].pos.x, 5);
    ASSERT_EQ(arena->crystals[0].pos.y, 1);
    ASSERT_EQ(arena->crystals[1].pos.x, 1);
    ASSERT_EQ(arena->crystals[1].pos.y, 5);

    // Check spawn points
    ASSERT_EQ(arena->num_spawn_points, 2);
    ASSERT_EQ(arena->spawn_points[0].pos.x, 1);
    ASSERT_EQ(arena->spawn_points[0].pos.y, 2);
    ASSERT_EQ(arena->spawn_points[1].pos.x, 5);
    ASSERT_EQ(arena->spawn_points[1].pos.y, 4);

    // Check crystal count
    ASSERT_EQ(arena->num_crystals, 2);
}

TEST(test_arena_load_ascii) {
    Arena arena;
    bool result = arena_load_from_string(&arena, TEST_MAP_ASCII);
    ASSERT(result, "Failed to load arena from ascii string");
    test_arena_load(&arena);
}

TEST(test_arena_load_utf8) {
    Arena arena;
    bool result = arena_load_from_string(&arena, TEST_MAP_UTF8);
    ASSERT(result, "Failed to load arena from utf8 string");
    test_arena_load(&arena);
}

TEST(test_arena_crystal) {
    Arena arena;
    arena_load_from_string(&arena, TEST_MAP_ASCII);

    int crystal_idx = arena_get_crystal_at(&arena, 5, 1);
    ASSERT(crystal_idx >= 0, "Crystal should exist at position");
    ASSERT(arena_crystal_available(&arena, crystal_idx), "Crystal should be available initially");

    arena_collect_crystal(&arena, crystal_idx);
    ASSERT(!arena_crystal_available(&arena, crystal_idx), "Crystal should not be available after collection");

    // Tick down the cooldown
    for (int i = 0; i < CRYSTAL_RESPAWN_TICKS; i++) {
        arena_tick_crystals(&arena);
    }
    ASSERT(arena_crystal_available(&arena, crystal_idx), "Crystal should respawn after cooldown");
}

// =============================================================================
// Player Tests
// =============================================================================

TEST(test_player_init) {
    Player player;
    Position spawn = {5, 5};
    player_init(&player, spawn);

    ASSERT_EQ(player.pos.x, 5);
    ASSERT_EQ(player.pos.y, 5);
    ASSERT_EQ(player.health, STARTING_HEALTH);
    ASSERT_EQ(player.energy, STARTING_ENERGY);
    ASSERT(player.alive, "Player should be alive after init");
    ASSERT(player_can_move(&player), "Player should be able to move after init");
    ASSERT(player_can_shoot(&player), "Player should be able to shoot after init");
}

TEST(test_player_damage) {
    Player player;
    Position spawn = {0, 0};
    player_init(&player, spawn);

    player_take_damage(&player, 1);
    ASSERT_EQ(player.health, 3);
    ASSERT(player.alive, "Player should be alive with health remaining");

    player_take_damage(&player, 3);
    ASSERT_EQ(player.health, 0);
    ASSERT(!player.alive, "Player should be dead at zero health");
}

TEST(test_player_cooldowns) {
    Player player;
    Position spawn = {0, 0};
    player_init(&player, spawn);

    ASSERT(player_can_move(&player), "Player should be able to move initially");
    player_start_move_cooldown(&player);
    ASSERT(!player_can_move(&player), "Player should not be able to move during cooldown");

    // Tick down cooldown
    for (int i = 0; i < MOVEMENT_COOLDOWN_TICKS; i++) {
        player_tick_cooldowns(&player);
    }
    ASSERT(player_can_move(&player), "Player should be able to move after cooldown expires");
}

TEST(test_player_energy) {
    Player player;
    Position spawn = {0, 0};
    player_init(&player, spawn);

    ASSERT_EQ(player.energy, 8);
    ASSERT(player_use_energy(&player, 1), "Should successfully use energy");
    ASSERT_EQ(player.energy, 7);

    // Use all energy
    for (int i = 0; i < 7; i++) {
        player_use_energy(&player, 1);
    }
    ASSERT_EQ(player.energy, 0);
    ASSERT(!player_use_energy(&player, 1), "Should fail to use energy when depleted");
}

// =============================================================================
// Combat Tests
// =============================================================================

TEST(test_combat_fire_laser_hit) {
    GameState state;
    game_init(&state, "1 . . 2 .");

    // Fire right from player 1
    LaserResult result = combat_fire_laser(&state, 0, DIR_RIGHT);

    ASSERT_EQ(result.hit_type, LASER_HIT_PLAYER);
    ASSERT_EQ(result.target_player, 1);
    ASSERT_EQ(result.hit_position.x, 3);
    ASSERT_EQ(result.hit_position.y, 0);
    ASSERT_EQ(result.pushback_to.x, 4);
    ASSERT_EQ(result.pushback_to.y, 0);
    ASSERT(!result.target_fragged, "Player should not be fragged");
}

TEST(test_combat_fire_laser_blocked_by_wall) {
    GameState state;
    game_init(&state, "1 # 2");
    LaserResult result = combat_fire_laser(&state, 0, DIR_RIGHT);
    ASSERT_EQ(result.hit_type, LASER_HIT_WALL);
    ASSERT_EQ(result.hit_position.x, 1);
    ASSERT_EQ(result.hit_position.y, 0);
    ASSERT_EQ(result.target_player, -1);
    ASSERT_EQ(result.pushback_to.x, -1);
    ASSERT_EQ(result.pushback_to.y, -1);
    ASSERT(!result.target_fragged, "Player should not be fragged");
}

TEST(test_combat_pushback) {
    GameState state;
    game_init(&state, "1 .");

    bool fragged = false;
    Position new_pos = combat_apply_pushback(&state, 1, DIR_RIGHT, 1, &fragged);

    ASSERT(!fragged, "Player should not be fragged by pushback into open space");
    ASSERT_EQ(new_pos.x, 1);
    ASSERT_EQ(new_pos.y, 0);
}

TEST(test_combat_pushback_into_wall) {
    GameState state;
    game_init(&state, "1 #");

    bool fragged = false;
    Position new_pos = combat_apply_pushback(&state, 1, DIR_RIGHT, 1, &fragged);

    ASSERT(!fragged, "Player should not be fragged when pushed into wall");
    // position should be unchanged
    ASSERT_EQ(new_pos.x, 0);
    ASSERT_EQ(new_pos.y, 0);
}

TEST(test_combat_pushback_into_void) {
    GameState state;
    game_init(&state, "1 x");

    bool fragged = false;
    Position new_pos = combat_apply_pushback(&state, 1, DIR_RIGHT, 1, &fragged);

    ASSERT(fragged, "Player should be fragged when pushed into void");
    // position should be void tile
    ASSERT_EQ(new_pos.x, 1);
    ASSERT_EQ(new_pos.y, 0);
}

// =============================================================================
// Game Tests
// =============================================================================

TEST(test_game_init) {
    GameState state;
    game_init(&state, TEST_MAP_ASCII);

    ASSERT_EQ(state.current_tick, 0);
    ASSERT_EQ(state.winner, -1);
    ASSERT(!state.game_over, "Game should not be over at start");

    // Players should be at spawn points
    ASSERT_EQ(state.players[0].pos.x, 1);
    ASSERT_EQ(state.players[0].pos.y, 2);
    ASSERT_EQ(state.players[1].pos.x, 5);
    ASSERT_EQ(state.players[1].pos.y, 4);
}

TEST(test_game_step_movement) {
    GameState state;
    game_init(&state, TEST_MAP_ASCII);

    // Player 0 moves right
    PlayerAction actions[2] = {
        {ACTION_RIGHT, ACTION_NOOP},
        {ACTION_NOOP, ACTION_NOOP}
    };

    game_step(&state, actions);

    ASSERT_EQ(state.players[0].pos.x, 2);
    ASSERT_EQ(state.players[0].pos.y, 2);
}

TEST(test_game_step_shooting) {
    GameState state;
    game_init(&state, TEST_MAP_ASCII);

    // Place players in line
    state.players[0].pos.x = 2;
    state.players[0].pos.y = 3;
    state.players[1].pos.x = 4;
    state.players[1].pos.y = 3;

    // Player 0 shoots right
    PlayerAction actions[2] = {
        {ACTION_NOOP, ACTION_RIGHT},
        {ACTION_NOOP, ACTION_NOOP}
    };

    StepInfo info = game_step(&state, actions);

    ASSERT(info.player_hit[1], "Player 1 should be hit by laser");
    ASSERT_EQ(state.players[1].health, 3);
    ASSERT_EQ(state.players[1].pos.x, 5);  // Pushed right
}

TEST(test_game_simultaneous_shoot) {
    GameState state;
    game_init(&state, TEST_MAP_ASCII);

    // Place players facing each other
    state.players[0].pos.x = 2;
    state.players[0].pos.y = 3;
    state.players[1].pos.x = 4;
    state.players[1].pos.y = 3;

    // Both shoot at each other
    PlayerAction actions[2] = {
        {ACTION_NOOP, ACTION_RIGHT},
        {ACTION_NOOP, ACTION_LEFT}
    };

    StepInfo info = game_step(&state, actions);

    // Both should be hit
    ASSERT(info.player_hit[0], "Player 0 should be hit in simultaneous shoot");
    ASSERT(info.player_hit[1], "Player 1 should be hit in simultaneous shoot");
    ASSERT_EQ(state.players[0].health, 3);
    ASSERT_EQ(state.players[1].health, 3);
}

TEST(test_game_movement_collision) {
    GameState state;
    game_init(&state, TEST_MAP_ASCII);

    // Place players adjacent
    state.players[0].pos.x = 2;
    state.players[0].pos.y = 3;
    state.players[1].pos.x = 4;
    state.players[1].pos.y = 3;

    // Both try to move to (3, 3)
    PlayerAction actions[2] = {
        {ACTION_RIGHT, ACTION_NOOP},
        {ACTION_LEFT, ACTION_NOOP}
    };

    game_step(&state, actions);

    // Both should stay in place
    ASSERT_EQ(state.players[0].pos.x, 2);
    ASSERT_EQ(state.players[1].pos.x, 4);
}

TEST(test_game_crystal_collection) {
    GameState state;
    game_init(&state, TEST_MAP_ASCII);

    // Use some energy first
    state.players[0].energy = 3;

    // Move player 0 to crystal position (1, 5)
    state.players[0].pos.x = 1;
    state.players[0].pos.y = 4;

    PlayerAction actions[2] = {
        {ACTION_DOWN, ACTION_NOOP},
        {ACTION_NOOP, ACTION_NOOP}
    };

    StepInfo info = game_step(&state, actions);

    ASSERT(info.crystal_collected[0], "Player 0 should have collected crystal");
    ASSERT_EQ(state.players[0].energy, MAX_ENERGY);

    // Crystal should be on cooldown now
    int crystal_idx = arena_get_crystal_at(&state.arena, 1, 5);
    ASSERT(!arena_crystal_available(&state.arena, crystal_idx), "Crystal should be on cooldown after collection");
}

TEST(test_game_frag_and_respawn) {
    GameState state;
    game_init(&state, TEST_MAP_ASCII);
    api_game_set_seed(42);  // For reproducible respawn

    // Place players
    state.players[0].pos.x = 2;
    state.players[0].pos.y = 3;
    state.players[1].pos.x = 4;
    state.players[1].pos.y = 3;
    state.players[1].health = 1;

    // Player 0 shoots player 1 who has 1 health
    PlayerAction actions[2] = {
        {ACTION_NOOP, ACTION_RIGHT},
        {ACTION_NOOP, ACTION_NOOP}
    };

    StepInfo info = game_step(&state, actions);

    ASSERT(info.player_fragged[1], "Player 1 should be fragged");
    ASSERT_EQ(state.players[0].score, 1);
    ASSERT(state.players[1].alive, "Player 1 should have respawned");
    ASSERT_EQ(state.players[1].health, MAX_HEALTH);
}

// =============================================================================
// API Tests
// =============================================================================

TEST(test_api_basic) {
    GameState state;
    api_game_init(&state, TEST_MAP_ASCII);

    ASSERT_EQ(api_get_arena_width(&state), 7);
    ASSERT_EQ(api_get_arena_height(&state), 7);
    ASSERT_EQ(api_get_player_health(&state, 0), 4);
    ASSERT_EQ(api_get_player_energy(&state, 0), 8);
    ASSERT(!api_is_game_over(&state), "Game should not be over after init");
}

TEST(test_api_step) {
    GameState state;
    api_game_init(&state, TEST_MAP_ASCII);

    // Player 0 moves right, player 1 does nothing
    int actions[4] = {ACTION_RIGHT, ACTION_NOOP, ACTION_NOOP, ACTION_NOOP};

    api_game_step(&state, actions);

    ASSERT_EQ(api_get_player_x(&state, 0), 2);
    ASSERT_EQ(api_get_current_tick(&state), 1);
}

// =============================================================================
// Main
// =============================================================================

int main(void) {
    printf("Running Arena Game Engine Tests\n");
    printf("================================\n\n");

    printf(COLOR_CYAN "Arena Tests:" COLOR_RESET "\n");
    RUN_TEST(test_arena_load_ascii);
    RUN_TEST(test_arena_load_utf8);
    RUN_TEST(test_arena_crystal);
    printf("\n");

    printf(COLOR_CYAN "Player Tests:" COLOR_RESET "\n");
    RUN_TEST(test_player_init);
    RUN_TEST(test_player_damage);
    RUN_TEST(test_player_cooldowns);
    RUN_TEST(test_player_energy);
    printf("\n");

    printf(COLOR_CYAN "Combat Tests:" COLOR_RESET "\n");
    RUN_TEST(test_combat_fire_laser_hit);
    RUN_TEST(test_combat_fire_laser_blocked_by_wall);
    RUN_TEST(test_combat_pushback);
    RUN_TEST(test_combat_pushback_into_wall);
    RUN_TEST(test_combat_pushback_into_void);
    printf("\n");

    printf(COLOR_CYAN "Game Tests:" COLOR_RESET "\n");
    RUN_TEST(test_game_init);
    RUN_TEST(test_game_step_movement);
    RUN_TEST(test_game_step_shooting);
    RUN_TEST(test_game_simultaneous_shoot);
    RUN_TEST(test_game_movement_collision);
    RUN_TEST(test_game_crystal_collection);
    RUN_TEST(test_game_frag_and_respawn);
    printf("\n");

    printf(COLOR_CYAN "API Tests:" COLOR_RESET "\n");
    RUN_TEST(test_api_basic);
    RUN_TEST(test_api_step);
    printf("\n");

    printf("================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}
