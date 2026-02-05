#ifndef ARENA_SCREENSHOT_H
#define ARENA_SCREENSHOT_H

#include "render.h"

// Take a screenshot and save it to the screenshots directory.
// Filename format: screenshots/YYYY-MM-DD_HH-MM-SS.png
// Returns 0 on success, -1 on failure.
int screenshot_save(RenderContext* ctx);

#endif // ARENA_SCREENSHOT_H
