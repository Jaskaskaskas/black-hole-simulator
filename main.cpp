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
  float vx, vy, vz;
  float r, phi;
  float dr, dphi;
  float L, E;
  float b;
  float dt;
  float angle;
  float brightness = 0.0f;
  bool active;
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

struct vec3 {
  float x, y, z;
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

  image img = {100, 100, std::vector<uint8_t>(img.width * img.height * 3, 20)};

  // The origin is at the center of the window
  blackhole bh = {0.0f, 0.0f, 50.0f};
  accretiondisk ad = {bh.sradius * 1.8f, bh.sradius * 3.0f, 100.0f};

  float c = 1.0f;
  float angle = 0.0f * M_PI / 180.0f;

  const float X = -250.0f;
  const float Y = -200.0f;
  const float Z = -200.0f;
  const float dy = 3.0f;
  const float dz = 3.0f;

  photons ps;
  for (int x = 0; x < img.width; x++) {
    for (int y = 0; y < img.height; y++) {
      photon p = {
          x,   y, X, Y + y * dy, Z + x * dz, c * cosf(angle), c * sinf(-angle),
          0.0f};
      ps.list.push_back(p);
      // printf("Created photon at (%f, %f, %f)\n", p.x, p.y, p.z);
    }
  }
  for (photon& p : ps.list) {
    printf("Initializing photon at (%f, %f, %f)\n", p.x, p.y, p.z);
    // p.angle = M_PI / 2.0f;
    printf("old location: (%f, %f, %f)\n", p.x, p.y, p.z);

    p.angle = atan2(p.y, p.z);

    float a = p.y * p.vz - p.z * p.vy;
    float b = p.z * p.vx - p.x * p.vz;
    float c = p.x * p.vy - p.y * p.vx;
    float norm = sqrtf(a * a + b * b + c * c);
    a /= norm;
    b /= norm;
    c /= norm;

    vec3 new_x_axis = {b, -a, 0.0f};
    vec3 new_y_axis = {a * c, b * c, -(a * a + b * b)};
    new_x_axis.x /=
        sqrtf(new_x_axis.x * new_x_axis.x + new_x_axis.y * new_x_axis.y +
              new_x_axis.z * new_x_axis.z);
    new_x_axis.y /=
        sqrtf(new_x_axis.x * new_x_axis.x + new_x_axis.y * new_x_axis.y +
              new_x_axis.z * new_x_axis.z);

    float new_x = new_x_axis.x * p.x + new_x_axis.y * p.y + new_x_axis.z * p.z;
    float new_y = new_y_axis.x * p.x + new_y_axis.y * p.y + new_y_axis.z * p.z;

    p.x = new_x;
    p.y = new_y;
    p.z = 0.0f;
    p.vx = new_x_axis.x * p.vx;
    p.vy = new_x_axis.y * p.vx;

    printf("new location: (%f, %f, %f)\n", p.x, p.y, p.z);
    printf("plane normal: (%f, %f, %f)\n", a, b, c);

    p.active = true;
    // printf("Photon angle: %f degrees\n", p.angle * 180.0f / M_PI);

    p.r = hypotf(p.x, p.y);
    // p.phi = acosf(p.x / sqrtf(p.x * p.x + p.y * p.y + p.z * p.z));
    p.phi = atan2(p.y, p.x);

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
    // #pragma omp parallel for
    for (auto it = ps.list.begin(); it != ps.list.end();) {
      if (!it->active) {
        ++it;
        continue;
      }
      photon& p = *it;

      if (p.idx < 0 || p.idx >= img.width || p.idy < 0 || p.idy >= img.height) {
        std::cerr << "Out of bounds: idx=" << p.idx << " idy=" << p.idy
                  << std::endl;
        p.active = false;
        continue;
      }

      if (p.r > ad.inner_r && p.r < ad.outer_r) {
        // Photon is in the accretion disk
        if (p.angle != 0.0f) {
          if (fabsf(fmodf(p.phi, M_PI)) < 10.0f / p.r) {
            // printf("the value %f\n", fmodf(p.phi, M_PI));
            p.brightness = ad.brightness;
            img.data[3 * (img.width * p.idx + p.idy)] = std::min(
                255, (int)(img.data[3 * (p.idx * img.width + p.idy) + 0] +
                           (uint8_t)p.brightness));
            // printf("Photon %d brightness increased to %f\n", p.idx *
            // img.width + p.idy, p.brightness); it = ps.list.erase(it);
            p.active = false;
            // continue;
          }
        } else {
          p.brightness = ad.brightness;
          img.data[3 * (img.width * p.idx + p.idy)] = std::min(
              255, (int)(img.data[3 * (p.idx * img.width + p.idy) + 0] +
                         (uint8_t)p.brightness));
          // printf("Photon %d brightness increased to %f\n",p.idx * img.width +
          // p.idy, p.brightness); it = ps.list.erase(it);
          p.active = false;
          // continue;
        }
      }

      if (p.r <= bh.sradius) {
        // Photon is within the black hole's Schwarzschild radius
        // std::cout << "Photon absorbed by black hole!" << std::endl;
        img.data[3 * (img.width * p.idx + p.idy)] = p.brightness;
        // it = ps.list.erase(it);
        p.active = false;
        // continue;
      }
      // p.t.head.push_front(std::make_pair(p.x, p.y));
      // if (p.t.head.size() > p.t.max_length) {
      //   p.t.head.pop_back();
      // }
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
      // running = running && p.r < 1000.0f;
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
    // SDL_Delay(1);
  }

  for (int x = 0; x < img.width; x++) {
    for (int y = 0; y < img.height; y++) {
      printf("%d ", (int)img.data[3 * (x * img.width + y)]);
      // printf("x: %d y: %d value: %d | \n", x, y,
      //        (int)img.data[3 * (y * img.width + x)]);
    }
    printf("\n");
  }

  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
