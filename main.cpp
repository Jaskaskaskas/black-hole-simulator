#include <SDL2/SDL.h>

#include <deque>
#include <iostream>
#include <vector>

#include "SDL_tools.h"

struct photon {
  float x, y;
  float vx, vy;
  float r, phi;
};

struct trace {
  std::deque<photon> head;
  size_t max_length = 255;
};

int main(int argc, char* argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  const int WIN_W = 800;
  const int WIN_H = 600;

  SDL_Window* win =
      SDL_CreateWindow("SDL2 Simple Graphics", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, SDL_WINDOW_SHOWN);
  if (!win) {
    std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return 1;
  }

  SDL_Renderer* ren = SDL_CreateRenderer(
      win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!ren) {
    std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 1;
  }

  bool running = true;
  SDL_Event e;

  // moving rectangle state
  SDL_Rect rect{100, 100, 100, 100};
  int vx = 3;  // velocity x
  int vy = 2;  // velocity y

  // The origin is at the center of the window
  photon p = {0.0f, 0.0f, 1.0f, 0.0f};
  p.r = hypot(p.x, p.y);
  p.phi = atan2f(p.y, p.x);
  trace t;

  while (running) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = false;
      if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
      }
    }

    // update rectangle position
    rect.x += vx;
    rect.y += vy;
    if (rect.x <= 0 || rect.x + rect.w >= WIN_W) vx = -vx;
    if (rect.y <= 0 || rect.y + rect.h >= WIN_H) vy = -vy;

    t.head.push_front(p);
    if (t.head.size() > t.max_length) {
      t.head.pop_back();
    }

    p.x += p.vx;
    p.y += p.vy;
    p.r = hypot(p.x, p.y);
    p.phi = atan2f(p.y, p.x);

    // Clear with dark blue
    SDL_SetRenderDrawColor(ren, 10, 24, 74, 255);
    SDL_RenderClear(ren);

    // Draw a filled rectangle (orange)
    SDL_SetRenderDrawColor(ren, 255, 140, 0, 255);
    SDL_RenderFillRect(ren, &rect);

    // Draw a white border around the rectangle
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    SDL_RenderDrawRect(ren, &rect);

    // Draw photon
    SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
    drawCircle(ren, p.x + WIN_W / 2, p.y + WIN_H / 2, 10);

    int x = 0;
    for (photon pt : t.head) {
      SDL_SetRenderDrawColor(ren, 255 - x++, 0, 0, 0);
      drawCircle(ren, pt.x + WIN_W / 2, pt.y + WIN_H / 2, 3);
    }

    SDL_RenderPresent(ren);

    SDL_Delay(16);  // ~60 FPS
  }

  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
