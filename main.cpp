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

  image img = {1000, 1000,
               std::vector<float>(img.width * img.height * 3, 0.0f)};

  // The origin is at the center of the window
  blackhole bh = {0.0f, 0.0f, 15.0f};
  accretiondisk ad = {bh.sradius * 1.8f, bh.sradius * 3.0f, 80.0f};  // 190

  float c = 1.0f;
  float angle = 0.0f * M_PI / 180.0f;

  const float X = -250.0f;
  const float Y = -150.0f;
  const float Z = -100.0f;
  const float dy = 0.2f;
  const float dz = 0.2f;

  const float limit = sqrtf(X * X + Y * Y + Z * Z);

  photons ps;
  for (int x = 0; x < img.width; x++) {
    for (int y = 0; y < img.height; y++) {
      float startX = X;
      float startY = Y + (y * dy);
      float startZ = Z + (x * dz);

      photon p = {};
      p.idx = x;
      p.idy = y;
      p.x = startX;
      p.y = startY;
      p.z = startZ;

      // Point them toward the Black Hole (positive X direction)
      p.vx = 1.0f;
      p.vy = 0.2f;
      p.vz = 0.0f;

      ps.list.push_back(p);
    }
  }
  for (photon& p : ps.list) {
    // The curvature of the path the light takes is calculated in a 2D plane.
    // Here we calculate the 2D local basis vectors for each photon.

    // 1. Original 3D vectors
    vec3 pos3D = {p.x, p.y, p.z};
    vec3 vel3D = {p.vx, p.vy, p.vz};
    vec3 L_vec = {
        pos3D.y * vel3D.z - pos3D.z * vel3D.y,
        pos3D.z * vel3D.x - pos3D.x * vel3D.z,
        pos3D.x * vel3D.y - pos3D.y * vel3D.x};  // The plane's normal vector

    // 2. u1 is just the normalized starting position vector

    float r_init =
        sqrtf(pos3D.x * pos3D.x + pos3D.y * pos3D.y + pos3D.z * pos3D.z);
    p.u1 = {pos3D.x / r_init, pos3D.y / r_init, pos3D.z / r_init};

    // 3. u2 must be perpendicular to both L and u1 to complete the plane
    // u2 = (L x u1) / |L x u1|
    vec3 u2_raw = {L_vec.y * p.u1.z - L_vec.z * p.u1.y,
                   L_vec.z * p.u1.x - L_vec.x * p.u1.z,
                   L_vec.x * p.u1.y - L_vec.y * p.u1.x};

    float u2_mag =
        sqrtf(u2_raw.x * u2_raw.x + u2_raw.y * u2_raw.y + u2_raw.z * u2_raw.z);
    if (u2_mag < 1e-6f) {
      // Fallback for edge cases where pos and vel are parallel (straight into
      // BH)
      p.u2 = {0, 1, 0};
    } else {
      p.u2 = {u2_raw.x / u2_mag, u2_raw.y / u2_mag, u2_raw.z / u2_mag};
    }

    // 4. Project 3D position/velocity into 2D basis

    float local_x = (pos3D.x * p.u1.x + pos3D.y * p.u1.y + pos3D.z * p.u1.z);
    float local_y = (pos3D.x * p.u2.x + pos3D.y * p.u2.y + pos3D.z * p.u2.z);

    float local_vx = (vel3D.x * p.u1.x + vel3D.y * p.u1.y + vel3D.z * p.u1.z);
    float local_vy = (vel3D.x * p.u2.x + vel3D.y * p.u2.y + vel3D.z * p.u2.z);

    // 5. Now calculate Polar coordinates
    p.r = sqrtf(local_x * local_x + local_y * local_y);
    p.phi = atan2f(local_y, local_x);
    p.L = local_x * local_vy - local_y * local_vx;

    p.active = true;
    float speed = sqrtf(local_vx * local_vx + local_vy * local_vy);
    p.E = speed;
    p.b = p.L / p.E;

    float vr = (local_x * local_vx + local_y * local_vy) / p.r;
    float dr_mag = sqrtf(std::max(
        0.0f, 1.0f - (1.0f - bh.sradius / p.r) * (p.b * p.b / (p.r * p.r))));

    p.dr = vr > 0 ? dr_mag : -dr_mag;
    p.dphi = p.b / (p.r * p.r);

    p.dt = 1.0f / (1.0f - bh.sradius / p.r);
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
        float r2 = p.r * p.r;
        float r3 = r2 * p.r;

        float d2r =
            (bh.sradius * p.L * p.L) / (r2 * r2) - (bh.sradius / (2 * r2));

        p.dr += d2r * dlambda;
        p.r += p.dr * dlambda;
        p.phi += (p.L / r2) * dlambda;

        // Calculate global coordinates from local coordinates
        float worldX = p.r * cosf(p.phi) * p.u1.x + p.r * sinf(p.phi) * p.u2.x;
        float worldY = p.r * cosf(p.phi) * p.u1.y + p.r * sinf(p.phi) * p.u2.y;
        float worldZ = p.r * cosf(p.phi) * p.u1.z + p.r * sinf(p.phi) * p.u2.z;

        float old_y = p.y;

        p.x = worldX;
        p.y = worldY;
        p.z = worldZ;
      } else {
        // Non-relativistic initialization
        float old_r = p.r;
        p.x += p.vx;
        p.y += p.vy;
        p.z += p.vz;
        p.r = sqrtf(p.x * p.x + p.y * p.y + p.z * p.z);
        p.dr = p.r - old_r;
      }

      if (p.dr > 0.0f && p.r > limit) {
        // float background_brightness = -99.0f;  // background brightness
        float background_brightness = 1000.0f;  // background brightness
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
        if (p.y <= 2.0f && p.y >= -2.0f) {  // Thin disk approximation
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
    if (img.data[i] == -99.0f) {
      uint8_t value = static_cast<uint>(log2((dif) / 2) * 0.0f);
      balance_img.data[i] = value;
      balance_img.data[i + 1] = value;
      balance_img.data[i + 2] = value;
    } else {
      float norm = log2(((img.data[i] - min) / dif) + 1);
      uint8_t val = static_cast<uint>(norm * 255.0f);
      balance_img.data[i] = val;
      balance_img.data[i + 1] = val;
      balance_img.data[i + 2] = val;
    }
  }

  save_ppm(balance_img, "output.ppm");

  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
