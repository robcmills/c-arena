### Prompt 1

This project is a simple 2D tile-based game.
The core game logic is already implemented in c.
What we need to do now is implement a renderer using SDL.
Then render function should accept a GameState and render it to the screen.

Let's start with a simple harness that just initializes some hard-coded game state (game_init.c) and renders it.

### Prompt 2

This project is a simple 2D tile-grid-based arena combat game, written in c.
The core game logic is already implemented,
and we have rendering using SDL2 working.

But currently the rendering is very basic,
we are just rendering colored rectangles, triangles and circles.
Let's start refactoring and improving this to render proper game sprites.
Put together a plan and detailed prompt for how to do this, that involves:

- loading a sprite sheet from a file
- parsing the sprite sheet to extract indexed sprites
- rendering the arena tiles, players and energy crystals as sprites
- no animations yet, just static sprites
