#ifndef GRAPHICS_TOOLS_H
#define GRAPHICS_TOOLS_H

#include <SDL2/SDL.h>

#include <iostream>
#include <vector>

struct image {
  int width, height;
  std::vector<float> data;  // RGB format
};

void drawCircle(SDL_Renderer* renderer, int32_t cx, int32_t cy, int32_t radius);
void save_ppm(const struct image& img, const char* filename);

#endif
