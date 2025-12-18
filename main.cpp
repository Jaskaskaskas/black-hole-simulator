#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>
#include <deque>
#include <iostream>
#include <vector>

#include "SDL_tools.h"

const float dlambda = 0.1f;
struct trace {
  std::deque<std::pair<float, float>> head;
  size_t max_length = 255;
};
struct photon {
  float x, y;
  float vx, vy;
  float r, phi;
  float dr, dphi;
  float L, E;
  float b;
  float dt;
  trace t;
};

struct photons {
  std::vector<photon> list;
};

struct blackhole {
  float x, y;
  float sradius;
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

  // The origin is at the center of the window
  blackhole bh = {0.0f, 0.0f, 50.0f};

  photon p = {-200.0f, 200.0f, -100.0f, 0.0f};
  p.r = hypotf(p.x, p.y);
  p.phi = atan2f(p.y, p.x);

  printf("vx: %f, vy: %f\n", p.vx, p.vy);
  if (p.vy <= 1e-6f && p.vy >= -1e-6f)
    p.vy = 1e-6f;  // Prevent division by zero
  float k = -p.vy / p.vx;
  printf("k: %f\n", k);
  float c = p.y - k * p.x;
  printf("c: %f\n", c);
  p.b = std::abs(c) / sqrtf(p.vx * p.vx + p.vy * p.vy);
  p.L = p.b;
  p.E = 1.0f;

  p.dphi = p.b / (p.r * p.r);
  // p.dr = sqrtf(p.E * p.E - (1.0f - 2.0f / p.r) * (p.b * p.b / (p.r * p.r)));
  p.dr = -sqrtf(1.0f - (1.0f - bh.sradius / p.r) * (p.b * p.b / (p.r * p.r)));
  p.dt = 1.0f / (1.0f - bh.sradius / p.r);

  photons ps;
  ps.list.push_back(p);

  while (running) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = false;
      if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
      }
    }
    for (auto it = ps.list.begin(); it != ps.list.end();) {
      photon& p = *it;
      if (p.r <= bh.sradius) {
        // Photon is within the black hole's Schwarzschild radius
        std::cout << "Photon absorbed by black hole!" << std::endl;
        it = ps.list.erase(it);
        continue;
      }

      p.t.head.push_front(std::make_pair(p.x, p.y));
      if (p.t.head.size() > p.t.max_length) {
        p.t.head.pop_back();
      }
      printf("dr: %f, r: %f, phi: %f, b: %f\n", p.dr, p.r, p.phi, p.b);
      p.dphi = p.b / (p.r * p.r);
      if (p.dr > 1e-9f) {
        p.dr = p.dr =
            sqrtf(1.0f - (1.0f - bh.sradius / p.r) * (p.b * p.b / (p.r * p.r)));
      } else {
        p.dr = -sqrtf(1.0f -
                      (1.0f - bh.sradius / p.r) * (p.b * p.b / (p.r * p.r)));
      }
      p.dt = 1.0f / (1.0f - bh.sradius / p.r);

      p.r += p.dr * p.dt;
      p.phi += p.dphi * p.dt;

      p.x = p.r * cosf(p.phi);
      p.y = p.r * sinf(p.phi);

      ++it;
    }

    // SDL_SetRenderDrawColor(ren, 10, 24, 74, 255);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    // Draw photon
    SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);

    drawCircle(ren, bh.x + WIN_W / 2, bh.y + WIN_H / 2, bh.sradius);

    for (photon p : ps.list) {
      drawCircle(ren, p.x + WIN_W / 2, p.y + WIN_H / 2, 10);
      int x = 0;
      for (std::pair<float, float> pt : p.t.head) {
        int alpha = std::max(0, 255 - x);
        SDL_SetRenderDrawColor(ren, alpha, 0, 0, 255);
        drawCircle(ren, pt.first + WIN_W / 2, pt.second + WIN_H / 2, 3);
        x++;
      }
    }

    SDL_RenderPresent(ren);

    SDL_Delay(16);  // ~60 FPS
  }

  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
