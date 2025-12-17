#ifndef SDL_TOOLS_H
#define SDL_TOOLS_H

#include <SDL2/SDL.h>

void drawCircle(SDL_Renderer* renderer, int32_t cx, int32_t cy, int32_t radius);

#endif
void drawCircle(SDL_Renderer* renderer, int32_t cx, int32_t cy,
                int32_t radius) {
  int32_t x = radius - 1;
  int32_t y = 0;
  int32_t err = 1 - x;

  while (x >= y) {
    SDL_RenderDrawPoint(renderer, cx + x, cy + y);
    SDL_RenderDrawPoint(renderer, cx - x, cy + y);
    SDL_RenderDrawPoint(renderer, cx + x, cy - y);
    SDL_RenderDrawPoint(renderer, cx - x, cy - y);
    SDL_RenderDrawPoint(renderer, cx + y, cy + x);
    SDL_RenderDrawPoint(renderer, cx - y, cy + x);
    SDL_RenderDrawPoint(renderer, cx + y, cy - x);
    SDL_RenderDrawPoint(renderer, cx - y, cy - x);

    y++;
    if (err < 0) {
      err += 2 * y + 1;
    } else {
      x--;
      err += 2 * (y - x) + 1;
    }
  }
}