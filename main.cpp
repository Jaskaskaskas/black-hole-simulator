#include <SDL2/SDL.h>
#include <omp.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include "graphics_tools.h"
#include "physics.h"

const float dlambda = 0.2f;  // Smaller values yield more accurate results but
                             // take longer
const int render =
    0;  // set to 1 enables rendering, 0 disables rendering for faster execution
const int relativity =
    1;  // set to 1 enables relativistic effects, 0 disables them

int main(int argc, char* argv[]) {
  printf("Number of threads: %d\n", omp_get_max_threads());

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

  image img = {100, 100, std::vector<float>(img.width * img.height * 3, 0.0f)};

  // The origin is at the center of the window
  blackhole bh = {0.0f, 0.0f, 15.0f};
  accretiondisk ad = {bh.sradius * 1.8f, bh.sradius * 3.0f, 80.0f, 4.0f};

  vec3 camera = {-250.0f, -50.0f, 0.0f};
  float camera_theta = 0.0f;  // in degrees in the x-z plane
  float camera_phi = 10.0f;   // in degrees in the x-y plane

  float fov = 70.0f;  // degrees

  camera_theta = M_PI * (camera_theta / 180.0f);  // transform to radians
  camera_phi = M_PI * (camera_phi / 180.0f);
  fov = M_PI * (fov / 180.0f);

  const float limit =
      sqrtf(camera.x * camera.x + camera.y * camera.y + camera.z * camera.z);

  photons ps;
  for (int x = 0; x < img.width; x++) {
    for (int y = 0; y < img.height; y++) {
      photon p = {};
      p.idx = x;
      p.idy = y;
      p.x = camera.x;
      p.y = camera.y;
      p.z = camera.z;

      // Point them toward the Black Hole (positive X direction)

      vec3 dir = photon_direction(x, y, camera_theta, camera_phi, fov,
                                  img.width, img.height);

      p.vx = dir.x;
      p.vy = dir.y;
      p.vz = dir.z;
      ps.list.push_back(p);
    }
  }

  int error = 0;
  for (photon& p : ps.list) {
    error = error || initialize_photon(p, bh);
  }

  if (error) {
    printf("An error occured when initializing photons");
    return 1;
  }

  while (running) {
    bool any_active = false;
    for (photon& p : ps.list) {
      any_active = any_active || p.active;
    }
    running = any_active;

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = false;
      if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
      }
    }
#pragma omp parallel for
    for (size_t i = 0; i < ps.list.size(); i++) {
      photon& p = ps.list[i];
      if (!p.active) continue;

      if (relativity) {
        relativistic_simulation(p, bh, dlambda);
      } else {
        non_relativistic_simulation(p);
      }

      if (p.dr > 0.0f && p.r > limit) {  // Checks for escaping photons
        float background_brightness = 1000.0f;
        img.data[3 * (img.width * p.idy + p.idx)] =
            img.data[3 * (img.width * p.idy + p.idx)] + background_brightness;
        p.active = false;
        continue;
      }

      if (p.r <= bh.sradius) {
        // Photon is within the black hole's Schwarzschild radius
        // img.data[3 * (img.width * p.idy + p.idx)] = p.brightness;
        p.active = false;
      }

      if (p.r > ad.inner_r && p.r < ad.outer_r) {
        // Photon has the correct distance to be in the accretion disk
        // if ((old_y < 0.0f && p.y >= 0.0f) ||
        //    (old_y > 0.0f &&
        //     p.y <= 0.0f)) {  // The accretion disk is on the xz-plane, where
        // y = 0. Thus crossing y = 0 means crossing the disk
        if (p.y <= ad.thickness / 2.0f &&
            p.y >= -ad.thickness / 2.0f) {  // Thin disk approximation
          // TODO: add opaque parts to the disk (could be very non-trivial)
          float temperature = pos_to_brightness(p.x, p.z, p.r, ad.inner_r,
                                                ad.outer_r, ad.brightness);
          img.data[3 * (img.width * p.idy + p.idx)] =
              img.data[3 * (img.width * p.idy + p.idx)] + temperature;
          // p.active = false;
        }
      }
    }
    if (render) {
      SDL_SetRenderDrawColor(ren, 10, 24, 74, 255);
      // SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
      SDL_RenderClear(ren);
      // // Draw photon
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

      // SDL_Delay(16);  // ~60 FPS
      // SDL_Delay(1);
    }
  }

  // Crude scaling of the images brightness
  image balance_img = {img.width, img.height,
                       std::vector<float>(img.width * img.height * 3, 0)};

  float max = img.data[0];
  float min = img.data[0];
  for (size_t i = 0; i < img.data.size(); i += 3) {
    if (img.data[i] > max) max = img.data[i];
    if (img.data[i] < min) min = img.data[i];
  }
  float dif = max - min;
  printf("Image max: %f min: %f\n", max, min);
  for (size_t i = 0; i < img.data.size(); i += 3) {
    float norm = log2(((img.data[i] - min) / dif) + 1);
    uint8_t val = static_cast<uint>(norm * 255.0f);
    balance_img.data[i] = val;
    balance_img.data[i + 1] = val;
    balance_img.data[i + 2] = val;
  }

  save_ppm(balance_img, "output.ppm");

  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
