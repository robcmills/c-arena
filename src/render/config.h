#ifndef ARENA_CONFIG_H
#define ARENA_CONFIG_H

typedef struct {
    int scale;
} GameConfig;

// Load config from file. Returns 0 on success, -1 on failure.
int config_load(GameConfig* cfg, const char* path);

// Load default config (fallback)
void config_load_defaults(GameConfig* cfg);

#endif // ARENA_CONFIG_H
