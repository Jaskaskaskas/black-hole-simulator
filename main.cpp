#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>
#include <deque>
#include <iostream>
#include <vector>

#include "SDL_tools.h"

inline float sign(float x) {
  return (x > 0.0f) ? 1.0f : ((x < 0.0f) ? -1.0f : 0.0f);
}

const float dlambda = 1.0f;
struct trace {
  std::deque<std::pair<float, float>> head;
  size_t max_length = 255;
};
struct photon {
  int idx, idy;
  float x, y, z;
  float vx, vy;
  float r, phi;
  float dr, dphi;
  float L, E;
  float b;
  float dt;
  float angle;
  float brightness = 0.0f;
  trace t;
};

struct accretiondisk {
  float inner_r, outer_r;
  float brightness;
};

struct photons {
  std::vector<photon> list;
};

struct blackhole {
  float x, y;
  float sradius;
};

struct image {
  int width, height;
  std::vector<uint8_t> data;  // RGB format
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

  image img = {15, 15, std::vector<uint8_t>(img.width * img.height * 3, 20)};

  // The origin is at the center of the window
  blackhole bh = {0.0f, 0.0f, 50.0f};
  accretiondisk ad = {bh.sradius * 1.5f, bh.sradius * 4.0f, 100.0f};

  float c = 1.0f;
  float angle = 0.0f * M_PI / 180.0f;

  photons ps;
  for (int x = 0; x < img.width; x++) {
    for (int y = 0; y < img.height; y++) {
      photon p = {x,
                  y,
                  -200.0f,
                  80.0f + y * 10.0f,
                  80.0f + x * 10.0f,
                  c * cosf(angle),
                  c * sinf(-angle)};
      ps.list.push_back(p);
      printf("Created photon at (%f, %f, %f)\n", p.x, p.y, p.z);
    }
  }
  // photon p1 = {0, 0, -200.0f, 80.0f, c * cosf(angle), c * sinf(-angle)};
  // photon p2 = {0, 1, -200.0f, 90.0f, c * cosf(angle), c * sinf(-angle)};
  // photon p3 = {0, 2, -200.0f, 100.0f, c * cosf(angle), c * sinf(-angle)};
  // photon p4 = {0, 3, -200.0f, 110.0f, c * cosf(angle), c * sinf(-angle)};
  // photon p5 = {0, 4, -200.0f, 120.0f, c * cosf(angle), c * sinf(-angle)};
  // photon p6 = {0, 5, -200.0f, 130.0f, c * cosf(angle), c * sinf(-angle)};
  // photon p7 = {0, 6, -200.0f, 140.0f, c * cosf(angle), c * sinf(-angle)};
  // photon p8 = {0, 7, -200.0f, 150.0f, c * cosf(angle), c * sinf(-angle)};
  // photon p9 = {0, 8, -200.0f, 160.0f, c * cosf(angle), c * sinf(-angle)};
  // photon p10 = {0, 9, -200.0f, 170.0f, c * cosf(angle), c * sinf(-angle)};

  // photon p11 = {0, 10, -200.0f, 180.0f, c * cosf(angle), c * sinf(-angle)};
  // photon p12 = {0, 11, -200.0f, 190.0f, c * cosf(angle), c * sinf(-angle)};
  // photon p13 = {0, 12, -200.0f, 200.0f, c * cosf(angle), c * sinf(-angle)};
  // photon p14 = {0, 13, -200.0f, 210.0f, c * cosf(angle), c * sinf(-angle)};
  // photon p15 = {0, 14, -200.0f, 220.0f, c * cosf(angle), c * sinf(-angle)};

  // ps.list.push_back(p1);
  // ps.list.push_back(p2);
  // ps.list.push_back(p3);
  // ps.list.push_back(p4);
  // ps.list.push_back(p5);
  // ps.list.push_back(p6);
  // ps.list.push_back(p7);
  // ps.list.push_back(p8);
  // ps.list.push_back(p9);
  // ps.list.push_back(p10);
  // ps.list.push_back(p11);
  // ps.list.push_back(p12);
  // ps.list.push_back(p13);
  // ps.list.push_back(p14);
  // ps.list.push_back(p15);

  for (photon& p : ps.list) {
    // p.angle = M_PI / 2.0f;
    p.angle = atan2(p.y, p.z);
    printf("Photon angle: %f degrees\n", p.angle * 180.0f / M_PI);

    p.r = hypotf(p.x, p.y);
    p.phi = atan2f(p.y, p.x);
    p.L = p.x * p.vy - p.y * p.vx;
    float speed = sqrtf(p.vx * p.vx + p.vy * p.vy);
    p.E = speed;
    p.b = p.L / p.E;

    float vr = (p.x * p.vx + p.y * p.vy) / p.r;
    float dr_mag = sqrtf(std::max(
        0.0f, 1.0f - (1.0f - bh.sradius / p.r) * (p.b * p.b / (p.r * p.r))));

    p.dr = vr > 0 ? dr_mag : -dr_mag;
    p.dphi = p.b / (p.r * p.r);

    p.dt = 1.0f / (1.0f - bh.sradius / p.r);
  }

  while (running) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = false;
      if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
      }
    }
    for (auto it = ps.list.begin(); it != ps.list.end();) {
      photon& p = *it;
      if (p.r > ad.inner_r && p.r < ad.outer_r) {
        // Photon is in the accretion disk
        if (p.angle != 0.0f) {
          if (fabsf(fmodf(p.phi, M_PI)) < 10.0f / p.r) {
            printf("the value %f\n", fmodf(p.phi, M_PI));
            p.brightness = ad.brightness;
            img.data[img.width * p.idx + p.idy] =
                std::min(255, (int)(img.data[3 * (0 * img.width + 0) + 0] +
                                    (uint8_t)p.brightness));
            printf("Photon %d brightness increased to %f\n", p.idx,
                   p.brightness);
            it = ps.list.erase(it);
            continue;
          }
        } else {
          p.brightness = ad.brightness;
          img.data[img.width * p.idx + p.idy] =
              std::min(255, (int)(img.data[3 * (0 * img.width + 0) + 0] +
                                  (uint8_t)p.brightness));
          printf("Photon %d brightness increased to %f\n", p.idx, p.brightness);
          it = ps.list.erase(it);
          continue;
        }
      }

      if (p.r <= bh.sradius) {
        // Photon is within the black hole's Schwarzschild radius
        std::cout << "Photon absorbed by black hole!" << std::endl;
        img.data[img.width * p.idx + p.idy] = p.brightness;
        it = ps.list.erase(it);
        continue;
      }
      p.t.head.push_front(std::make_pair(p.x, p.y));
      if (p.t.head.size() > p.t.max_length) {
        p.t.head.pop_back();
      }
      // printf("dr: %f, r: %f, phi: %f, b: %f\n", p.dr, p.r, p.phi, p.b);

      float d2r = -bh.sradius / (2.0f * p.r * p.r) +
                  (1.0f - bh.sradius / p.r) * p.L * p.L / (p.r * p.r * p.r);

      // p.dt = 1.0f / (1.0f - bh.sradius / p.r);
      p.dr += d2r * dlambda;  // Velocity evolves
      p.r += p.dr * dlambda;  // Position evolves

      p.dphi = p.L / (p.r * p.r);
      p.phi += p.dphi * dlambda;

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
    SDL_RenderDrawLine(ren, (int)ad.inner_r + WIN_W / 2, 0 + WIN_H / 2,
                       (int)ad.outer_r + WIN_W / 2, WIN_H / 2);

    SDL_RenderDrawLine(ren, -(int)ad.inner_r + WIN_W / 2, 0 + WIN_H / 2,
                       -(int)ad.outer_r + WIN_W / 2, WIN_H / 2);

    for (photon p : ps.list) {
      SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
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

  for (int x = 0; x < img.width; x++) {
    for (int y = 0; y < img.height; y++) {
      printf("%d ", (int)img.data[3 * (y * img.width + x) + 0]);
    }
    printf("\n");
  }

  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
