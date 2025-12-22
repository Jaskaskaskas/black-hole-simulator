#ifndef SDL_TOOLS_H
#define SDL_TOOLS_H

#include <SDL2/SDL.h>

#include <iostream>

struct image {
  int width, height;
  std::vector<uint8_t> data;  // RGB format
};

void drawCircle(SDL_Renderer* renderer, int32_t cx, int32_t cy, int32_t radius);
void save_ppm(const struct image& img, const char* filename);

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

void save_ppm(const image& img, const char* filename) {
  FILE* f = fopen(filename, "wb");
  if (!f) {
    std::cerr << "Error opening file: " << filename << std::endl;
    return;
  }

  // Write PPM header
  fprintf(f, "P6\n%d %d\n255\n", img.width, img.height);

  // Write pixel data (assuming RGB format)
  fwrite(img.data.data(), 1, img.width * img.height * 3, f);

  fclose(f);
  std::cout << "Saved image to " << filename << std::endl;
}
