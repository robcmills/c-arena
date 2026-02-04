# High Level Goal

Make a game and train an AI to play it.
The game is a 2D, multi-player, top down, tile-grid-based, arena combat game.

## Architecture

I want to train the ai with Stable-Baselines3 
but implement the core game logic in C (for fast simulation)
and render it with SDL

Implementation should keep these goals in mind, with a clear separation of concerns.

The core game engine should have an api that is compatible with the training environment.

for example:
```python
    def step(self, action):
        # Execute action, return new state, reward, done
        return observation, reward, done, info
```
or
```python
    def step(self, action):
        # Self-play: get opponent action from opponent policy
        opponent_action = self._get_opponent_action()
        
        actions = (ctypes.c_int * 2)(action, opponent_action)
        lib.game_step(ctypes.byref(self.state), actions)
        
        obs = self._get_obs()
        reward = self._get_reward()
        done = self.state.winner != -1
        
        return obs, reward, done, False, {}
```

and rendering should be separate from the core game logic.

## Core Game Logic

### Arena

Gameplay occurs in a 2D grid of tiles, called the "arena".

There are 3 types of tiles: floor, wall, and void.
Floor tiles can be occupied by players.
Wall tiles:
  are impassable 
  block laser shots
  if a player is pushed into a wall, nothing happens, they remain on the adjacent floor tile
Void tiles:
  are empty
  lasers can pass through
  if a player moves or is pushed into a void, they instantly die
More types of tiles will be added later.

Initially, arenas are defined manually in code using 2d arrays of ascii characters.
They can be any size, but generally about 15x15 tiles.

For example:

```
× ■ ■ ■ ■ ■ ×
× □ □ □ □ ◆ ×
× ▷ □ □ □ □ ×
× □ □ □ □ □ ×
× □ □ □ □ ◁ ×
× ◆ □ □ □ □ ×
× ■ ■ ■ ■ ■ ×
```

where `×` is void, `■` is wall, `□` is floor and `◆` is an energy crystal.
and `▷` and `◁` are spawn points.

Tiles with an energy crystal on them are equivalent to floor tiles,
in that they can be occupied by players.

Spawn tiles are equivalent to floor tiles,
in that they can be occupied by players.

The full arena map is visible to all players at all times,
including the locations of all other players, entities and their collection cooldown timers.

### Entities

There is one type of map entity: energy crystals.
Energy crystals are placed in the arena and can be collected by players
to fully restore energy.
When an energy crystal is collected, it can not be collected again for a set amount of time.
A player can collect an energy crystal by moving onto the tile containing it.
More types of entities will be added later.

### Players

Each player controls a single character that can move in the cardinal directions
(up, down, left, right) and shoot a "laser" in the cardinal directions.

Movement is grid-based, so a movement action will move a player one tile.
Movement has a cooldown, and can only be performed once per set amount of time.

Only one player can occupy a tile at a time. If multiple players try to move onto the same tile, the player who moved first will occupy the target tile and the other players' movement will be cancelled.
If multiple players try to move onto the same tile at the same time, all players' movement will be cancelled.

Players can move and shoot simultaneously in the same step,
however, shooting is resolved before movement.
So if a player A shoots player B, and player B moves in the same step,
player B will be damaged and knocked back, and player B's movement will be cancelled.
If two players shoot each other simultaneously, both will take damage and be knocked back,
and their movements (if any) will be cancelled.

Each player starts 4 health points and 8 energy.
Each player has a maximum of 4 health points and 8 energy.

When a player reaches 0 health, they are "fragged", and the opponent's score is incremented by 1.
When a player is fragged, they immediately respawn in a random location (any floor tile) with full health and energy.
Gameplay continues for a set amount of time, or until any player reaches a set score.

A player's energy will regenerate automatically but very slowly.

### Weapon

The laser is a hitscan weapon that instantly deals 1 health damage and pushback of 1 tile to the target
regardless of distance, as long as the target is within line of sight.
The laser has a cooldown, and can only be fired once per set amount of time.
Firing the laser requires 1 energy.
If the player does not have any energy, they can not fire the laser.
The laser does not shoot through walls or players.
If 2 players shoot each other simultaneously, both will take damage and be knocked back,


## Specification

```yaml
timing:
  tick_rate_ms: 16.6667  # 60 ticks/frames per second
  episode_length_ms: 120000  # 120 seconds
  movement_cooldown_ms: 200  # 200ms
  laser_cooldown_ms: 200  # 200ms
  crystal_respawn_ms: 8000  # 8 seconds

arena:
  dimensions: 15x15  # or variable?
  generation: loaded_from_file  # or procedural?
  void_behavior: instant_frag

player:
  max_health: 4
  max_energy: 8
  starting_health: 4
  starting_energy: 8
  energy_regen: slow
  movement_speed: 1  # tiles per turn
  collision: players_cannot_share_tiles

combat:
  laser_damage: 1
  laser_cooldown: 200  # milliseconds
  pushback_distance: 1  # tiles
  pushback_wall_behavior: stop_at_wall  # no extra damage
  pushback_void_behavior: instant_frag  # or blocked?
  simultaneous_hit: both_take_damage  # no priority

turn:
  resolution_order: 
    1: entities  # crystal_collection
    2: shooting  # both players shoot simultaneously
    3: pushback  # apply knockback from hits
    4: movement  # both players move simultaneously
  max_turns: 120  # episode length in seconds
  win_score: 8  # first to X frags

energy_crystals:
  count: 2  # per arena
  respawn_cooldown: 8  # seconds
  collection: move_onto_tile

energy:
  regen_cooldown_ms: 2000  # 1 energy per 2 seconds

action_space:
  type: multi_discrete
  shape: [5, 5]  # [move, shoot]
  move_actions: [noop, up, down, left, right]
  shoot_actions: [noop, up, down, left, right]
  # Note: can move AND shoot in same turn

observation_space:
  grid_channels:
    0: walls  # binary
    1: void   # binary  
    2: floor  # binary (redundant but explicit)
    3: crystals_available  # binary
    4: crystals_cooldown  # normalized 0-1 (0=available, 1=just collected)
    5: self_position  # binary
    6: opponent_position  # binary
  scalars:  # all normalized to 0-1
    - self_health  # /4
    - self_energy  # /8
    - self_laser_cooldown  # /max_cooldown
    - self_move_cooldown   # /max_cooldown
    - opponent_health
    - opponent_energy
    - opponent_laser_cooldown
    - opponent_move_cooldown

collision:
  swap_positions: blocked  # A↔B swap = both stay
  pushback_into_player: blocked  # pushed player stops
  simultaneous_same_tile: both_cancelled

spawn:
  initial: fixed_per_map  # map defines spawn points
  respawn: random_unoccupied_floor
  respawn_min_distance: 3  # Manhattan distance from opponent

laser:
  range: infinite_or_offscreen  # until obstacle
  self_damage: false
  origin: pre_movement_position  # shooting resolves before movement

crystals:
  placement: fixed_per_map  # defined in map file
  collection_phase: movement  # collected when stepping onto tile

rewards:
  frag_opponent: +1.0
  get_fragged: -1.0
  damage_dealt: +0.5  # optional shaping
  damage_taken: -0.5  # optional shaping
  crystal_collected: +0.25  # optional shaping
  timeout: 0.0  # draw
  stacking: true  # frag = damage + frag reward
```

---

## 🏗️ Implementation Plan Outline

```
Phase 1: Core Game Engine (C)
├── game_state.h - State structures
├── game_logic.c - Rules & simulation
├── game_api.c - External interface (init, step, reset)
└── tests/ - Unit tests for core logic

Phase 2: Python Bindings
├── bindings.py - ctypes wrapper
└── env.py - Gymnasium environment

Phase 3: Rendering (SDL)
├── renderer.c - SDL visualization
└── Can run independently of training

Phase 4: Training
├── train.py - SB3 training loop
├── self_play.py - Self-play wrapper
└── evaluate.py - Evaluation scripts
```

---

## Test Cases

```yaml
test_cases:
  - name: "simultaneous_shoot"
    setup: "A at (5,5), B at (8,5), both shoot toward each other"
    expected: "Both take 1 damage, A pushed to (4,5), B pushed to (9,5)"
    
  - name: "shoot_then_move"
    setup: "A at (5,5) shoots right and moves right, B at (6,5)"
    expected: "B takes damage, pushed to (7,5), A moves to (6,5)"
    
  - name: "pushback_into_void"
    setup: "A at (5,5) shoots right, B at (6,5), void at (7,5)"
    expected: "B fragged, respawns randomly"
    
  - name: "pushback_into_wall"
    setup: "A shoots B, wall behind B"
    expected: "B takes damage, stays in place"
    
  - name: "crystal_on_cooldown"
    setup: "A moves onto crystal that was collected 4 seconds ago"
    expected: "A occupies tile, crystal not collected (still on cooldown)"
    
  - name: "movement_collision"
    setup: "A and B both try to move to same empty tile"
    expected: "Both movements cancelled, both stay in place"
```

---

## 📁 Suggested File Structure Enhancement

```
arena_project/
├── src/
│   ├── core/           # C game engine
│   │   ├── types.h     # Structs, enums, constants
│   │   ├── arena.h/c   # Map loading, tile queries
│   │   ├── player.h/c  # Player state, actions
│   │   ├── combat.h/c  # Laser, damage, pushback
│   │   ├── entity.h/c  # Crystals, future entities
│   │   ├── game.h/c    # Main game loop, resolution order
│   │   └── api.h/c     # External interface (init/step/reset)
│   │
│   ├── render/         # SDL renderer (separate compilation)
│   │   ├── renderer.h/c
│   │   └── sprites/
│   │
│   └── python/         # Python bindings
│       ├── bindings.py # ctypes wrapper
│       ├── env.py      # Gymnasium environment
│       └── wrappers.py # Frame stacking, normalization, etc.
│
├── maps/
│   └── arena_01.txt    # Map definitions
│
├── training/
│   ├── train.py
│   ├── self_play.py
│   ├── callbacks.py
│   └── configs/
│
├── tests/
│   ├── test_combat.c
│   ├── test_movement.c
│   └── test_env.py
│
└── Makefile / CMakeLists.txt
```

