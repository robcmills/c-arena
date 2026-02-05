#include "render.h"
#include <stdio.h>

// Color definitions
static const Color COLOR_FLOOR   = {40, 40, 40, 255};
static const Color COLOR_WALL    = {100, 100, 100, 255};
static const Color COLOR_VOID    = {0, 0, 0, 255};
static const Color COLOR_CRYSTAL = {0, 200, 255, 255};
static const Color COLOR_CRYSTAL_COOLDOWN = {0, 80, 100, 255};
static const Color COLOR_PLAYER1 = {255, 100, 100, 255};
static const Color COLOR_PLAYER2 = {100, 100, 255, 255};
static const Color COLOR_PLAYER_DEAD = {80, 80, 80, 255};
static const Color COLOR_HUD_BG  = {20, 20, 20, 255};
static const Color COLOR_HEALTH  = {255, 50, 50, 255};
static const Color COLOR_ENERGY  = {50, 200, 255, 255};

static void set_draw_color(SDL_Renderer* renderer, Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
}

int render_init(RenderContext* ctx, int arena_width, int arena_height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    ctx->window_width = arena_width * TILE_SIZE;
    ctx->window_height = arena_height * TILE_SIZE + HUD_HEIGHT;

    ctx->window = SDL_CreateWindow(
        "Arena",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        ctx->window_width,
        ctx->window_height,
        SDL_WINDOW_SHOWN
    );

    if (!ctx->window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    ctx->renderer = SDL_CreateRenderer(
        ctx->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!ctx->renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
        return -1;
    }

    return 0;
}

void render_cleanup(RenderContext* ctx) {
    if (ctx->renderer) {
        SDL_DestroyRenderer(ctx->renderer);
        ctx->renderer = NULL;
    }
    if (ctx->window) {
        SDL_DestroyWindow(ctx->window);
        ctx->window = NULL;
    }
    SDL_Quit();
}

void render_arena(RenderContext* ctx, const Arena* arena) {
    for (int y = 0; y < arena->height; y++) {
        for (int x = 0; x < arena->width; x++) {
            SDL_Rect tile_rect = {
                x * TILE_SIZE,
                y * TILE_SIZE,
                TILE_SIZE - 1,
                TILE_SIZE - 1
            };

            switch (arena->tiles[y][x]) {
                case TILE_FLOOR:
                    set_draw_color(ctx->renderer, COLOR_FLOOR);
                    break;
                case TILE_WALL:
                    set_draw_color(ctx->renderer, COLOR_WALL);
                    break;
                case TILE_VOID:
                    set_draw_color(ctx->renderer, COLOR_VOID);
                    break;
            }

            SDL_RenderFillRect(ctx->renderer, &tile_rect);
        }
    }
}

void render_crystals(RenderContext* ctx, const Arena* arena) {
    for (int i = 0; i < arena->num_crystals; i++) {
        const Crystal* crystal = &arena->crystals[i];

        // Diamond shape for crystal
        int cx = crystal->pos.x * TILE_SIZE + TILE_SIZE / 2;
        int cy = crystal->pos.y * TILE_SIZE + TILE_SIZE / 2;
        int size = TILE_SIZE / 3;

        if (crystal->cooldown_ticks > 0) {
            set_draw_color(ctx->renderer, COLOR_CRYSTAL_COOLDOWN);
        } else {
            set_draw_color(ctx->renderer, COLOR_CRYSTAL);
        }

        // Draw diamond as 4 triangles (approximated with lines)
        SDL_Point points[5] = {
            {cx, cy - size},      // top
            {cx + size, cy},      // right
            {cx, cy + size},      // bottom
            {cx - size, cy},      // left
            {cx, cy - size}       // back to top
        };
        SDL_RenderDrawLines(ctx->renderer, points, 5);

        // Fill with smaller diamond
        for (int dy = -size + 1; dy < size; dy++) {
            int width = size - abs(dy);
            SDL_RenderDrawLine(ctx->renderer,
                cx - width, cy + dy,
                cx + width, cy + dy);
        }
    }
}

void render_players(RenderContext* ctx, const Player players[MAX_PLAYERS]) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        const Player* player = &players[i];

        int px = player->pos.x * TILE_SIZE + TILE_SIZE / 2;
        int py = player->pos.y * TILE_SIZE + TILE_SIZE / 2;
        int radius = TILE_SIZE / 3;

        Color color;
        if (!player->alive) {
            color = COLOR_PLAYER_DEAD;
        } else if (i == 0) {
            color = COLOR_PLAYER1;
        } else {
            color = COLOR_PLAYER2;
        }

        set_draw_color(ctx->renderer, color);

        // Draw filled circle (approximated)
        for (int dy = -radius; dy <= radius; dy++) {
            int dx = (int)SDL_sqrt(radius * radius - dy * dy);
            SDL_RenderDrawLine(ctx->renderer,
                px - dx, py + dy,
                px + dx, py + dy);
        }

        // Draw player number
        SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
        // Simple "1" or "2" using lines
        if (i == 0) {
            SDL_RenderDrawLine(ctx->renderer, px, py - 4, px, py + 4);
        } else {
            SDL_RenderDrawLine(ctx->renderer, px - 3, py - 4, px + 3, py - 4);
            SDL_RenderDrawLine(ctx->renderer, px + 3, py - 4, px + 3, py);
            SDL_RenderDrawLine(ctx->renderer, px - 3, py, px + 3, py);
            SDL_RenderDrawLine(ctx->renderer, px - 3, py, px - 3, py + 4);
            SDL_RenderDrawLine(ctx->renderer, px - 3, py + 4, px + 3, py + 4);
        }
    }
}

void render_hud(RenderContext* ctx, const GameState* state) {
    int hud_y = state->arena.height * TILE_SIZE;

    // HUD background
    SDL_Rect hud_rect = {0, hud_y, ctx->window_width, HUD_HEIGHT};
    set_draw_color(ctx->renderer, COLOR_HUD_BG);
    SDL_RenderFillRect(ctx->renderer, &hud_rect);

    // Divider line
    SDL_SetRenderDrawColor(ctx->renderer, 60, 60, 60, 255);
    SDL_RenderDrawLine(ctx->renderer, 0, hud_y, ctx->window_width, hud_y);

    // Player stats
    for (int i = 0; i < MAX_PLAYERS; i++) {
        const Player* player = &state->players[i];
        int base_x = i * (ctx->window_width / 2) + 10;
        int bar_y = hud_y + 10;

        // Player indicator
        Color player_color = (i == 0) ? COLOR_PLAYER1 : COLOR_PLAYER2;
        set_draw_color(ctx->renderer, player_color);
        SDL_Rect indicator = {base_x, bar_y, 10, 40};
        SDL_RenderFillRect(ctx->renderer, &indicator);

        // Health bar
        int health_x = base_x + 20;
        set_draw_color(ctx->renderer, COLOR_HEALTH);
        for (int h = 0; h < player->health; h++) {
            SDL_Rect health_rect = {health_x + h * 15, bar_y, 12, 15};
            SDL_RenderFillRect(ctx->renderer, &health_rect);
        }
        // Empty health slots
        SDL_SetRenderDrawColor(ctx->renderer, 60, 60, 60, 255);
        for (int h = player->health; h < MAX_HEALTH; h++) {
            SDL_Rect health_rect = {health_x + h * 15, bar_y, 12, 15};
            SDL_RenderDrawRect(ctx->renderer, &health_rect);
        }

        // Energy bar
        set_draw_color(ctx->renderer, COLOR_ENERGY);
        for (int e = 0; e < player->energy; e++) {
            SDL_Rect energy_rect = {health_x + e * 10, bar_y + 22, 8, 12};
            SDL_RenderFillRect(ctx->renderer, &energy_rect);
        }
        // Empty energy slots
        SDL_SetRenderDrawColor(ctx->renderer, 40, 80, 100, 255);
        for (int e = player->energy; e < MAX_ENERGY; e++) {
            SDL_Rect energy_rect = {health_x + e * 10, bar_y + 22, 8, 12};
            SDL_RenderDrawRect(ctx->renderer, &energy_rect);
        }

        // Score (simple number representation with blocks)
        int score_x = base_x + 120;
        SDL_SetRenderDrawColor(ctx->renderer, 200, 200, 200, 255);
        for (int s = 0; s < player->score && s < 8; s++) {
            SDL_Rect score_rect = {score_x + s * 8, bar_y + 5, 6, 30};
            SDL_RenderFillRect(ctx->renderer, &score_rect);
        }
    }

    // Game status
    if (state->game_over) {
        // Draw "GAME OVER" indicator at center
        int center_x = ctx->window_width / 2 - 30;
        SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 0, 255);
        SDL_Rect go_rect = {center_x, hud_y + 20, 60, 20};
        SDL_RenderDrawRect(ctx->renderer, &go_rect);
    }
}

void render_game(RenderContext* ctx, const GameState* state) {
    // Clear screen
    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ctx->renderer);

    // Render all layers
    render_arena(ctx, &state->arena);
    render_crystals(ctx, &state->arena);
    render_players(ctx, state->players);
    render_hud(ctx, state);

    // Present
    SDL_RenderPresent(ctx->renderer);
}
