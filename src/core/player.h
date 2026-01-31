#ifndef ARENA_PLAYER_H
#define ARENA_PLAYER_H

#include "types.h"

// Initialize a player at a spawn position
void player_init(Player* player, Position spawn_pos);

// Reset player to spawn state (after frag)
void player_respawn(Player* player, Position spawn_pos);

// Cooldown management
void player_tick_cooldowns(Player* player);
bool player_can_move(const Player* player);
bool player_can_shoot(const Player* player);

// Actions
void player_start_move_cooldown(Player* player);
void player_start_laser_cooldown(Player* player);

// Damage and healing
void player_take_damage(Player* player, int damage);
void player_restore_energy(Player* player, int amount);
bool player_use_energy(Player* player, int amount);  // returns false if not enough energy

// State queries
bool player_is_alive(const Player* player);

#endif // ARENA_PLAYER_H
