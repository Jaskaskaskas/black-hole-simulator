#ifndef SDL_TOOLS_H
#define SDL_TOOLS_H

#include <SDL2/SDL.h>

#include <iostream>

struct image {
  int width, height;
  std::vector<float> data;  // RGB format
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

  // Write PPM header (P6 is binary RGB)
  fprintf(f, "P6\n%d %d\n255\n", img.width, img.height);

  float max_val = 0.0f;
  for (float v : img.data) {
    if (v > max_val) max_val = v;
  }
  if (max_val == 0.0f) max_val = 1.0f;  // Prevent div by zero

  for (size_t i = 0; i < img.data.size(); i++) {
    // Normalize (0.0 to 1.0) then scale to (0 to 255)
    float norm = img.data[i] / max_val;
    uint8_t byte =
        static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, norm * 255.0f)));

    fputc(byte, f);
  }

  fclose(f);
  std::cout << "Saved image to " << filename << " (Max brightness: " << max_val
            << ")" << std::endl;
}