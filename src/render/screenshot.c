#include "screenshot.h"
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <SDL_image.h>

#define SCREENSHOTS_DIR "screenshots"

static int ensure_screenshots_dir(void) {
    struct stat st = {0};
    if (stat(SCREENSHOTS_DIR, &st) == -1) {
        #ifdef _WIN32
        if (mkdir(SCREENSHOTS_DIR) != 0) {
        #else
        if (mkdir(SCREENSHOTS_DIR, 0755) != 0) {
        #endif
            fprintf(stderr, "Failed to create screenshots directory\n");
            return -1;
        }
    }
    return 0;
}

static void generate_filename(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    snprintf(buffer, size, "%s/%04d-%02d-%02d_%02d-%02d-%02d.png",
             SCREENSHOTS_DIR,
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
}

int screenshot_save(RenderContext* ctx) {
    if (ensure_screenshots_dir() != 0) {
        return -1;
    }

    char filename[256];
    generate_filename(filename, sizeof(filename));

    // Get window surface dimensions
    int width = ctx->window_width;
    int height = ctx->window_height;

    // Create a surface to hold the screenshot
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32,
                                                 0x00FF0000,  // R mask
                                                 0x0000FF00,  // G mask
                                                 0x000000FF,  // B mask
                                                 0xFF000000); // A mask
    if (!surface) {
        fprintf(stderr, "Failed to create surface: %s\n", SDL_GetError());
        return -1;
    }

    // Read pixels from the renderer
    if (SDL_RenderReadPixels(ctx->renderer, NULL, SDL_PIXELFORMAT_ARGB8888,
                             surface->pixels, surface->pitch) != 0) {
        fprintf(stderr, "Failed to read pixels: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return -1;
    }

    // Save as PNG
    if (IMG_SavePNG(surface, filename) != 0) {
        fprintf(stderr, "Failed to save PNG: %s\n", IMG_GetError());
        SDL_FreeSurface(surface);
        return -1;
    }

    SDL_FreeSurface(surface);
    printf("Screenshot saved: %s\n", filename);
    return 0;
}
