#include "player.h"

void player_init(Player* player, Position spawn_pos) {
    player->pos = spawn_pos;
    player->health = STARTING_HEALTH;
    player->energy = STARTING_ENERGY;
    player->move_cooldown_ticks = 0;
    player->laser_cooldown_ticks = 0;
    player->energy_regen_ticks = ENERGY_REGEN_TICKS;
    player->score = 0;
    player->alive = true;
}

void player_respawn(Player* player, Position spawn_pos) {
    player->pos = spawn_pos;
    player->health = STARTING_HEALTH;
    player->energy = STARTING_ENERGY;
    player->move_cooldown_ticks = 0;
    player->laser_cooldown_ticks = 0;
    player->energy_regen_ticks = ENERGY_REGEN_TICKS;
    player->alive = true;
    // Note: score is NOT reset on respawn
}

void player_tick_cooldowns(Player* player) {
    if (player->move_cooldown_ticks > 0) {
        player->move_cooldown_ticks--;
    }
    if (player->laser_cooldown_ticks > 0) {
        player->laser_cooldown_ticks--;
    }

    // Energy regeneration
    if (player->energy < MAX_ENERGY) {
        player->energy_regen_ticks--;
        if (player->energy_regen_ticks <= 0) {
            player->energy++;
            player->energy_regen_ticks = ENERGY_REGEN_TICKS;
        }
    } else {
        // Reset regen timer when at max energy
        player->energy_regen_ticks = ENERGY_REGEN_TICKS;
    }
}

bool player_can_move(const Player* player) {
    return player->alive && player->move_cooldown_ticks == 0;
}

bool player_can_shoot(const Player* player) {
    return player->alive &&
           player->laser_cooldown_ticks == 0 &&
           player->energy > 0;
}

void player_start_move_cooldown(Player* player) {
    player->move_cooldown_ticks = MOVEMENT_COOLDOWN_TICKS;
}

void player_start_laser_cooldown(Player* player) {
    player->laser_cooldown_ticks = LASER_COOLDOWN_TICKS;
}

void player_take_damage(Player* player, int damage) {
    player->health -= damage;
    if (player->health <= 0) {
        player->health = 0;
        player->alive = false;
    }
}

void player_restore_energy(Player* player, int amount) {
    player->energy += amount;
    if (player->energy > MAX_ENERGY) {
        player->energy = MAX_ENERGY;
    }
    // Reset regen timer when energy is restored
    player->energy_regen_ticks = ENERGY_REGEN_TICKS;
}

bool player_use_energy(Player* player, int amount) {
    if (player->energy < amount) {
        return false;
    }
    player->energy -= amount;
    return true;
}

bool player_is_alive(const Player* player) {
    return player->alive;
}
