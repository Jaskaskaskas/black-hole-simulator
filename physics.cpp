
#include "physics.h"

#include <cmath>

float pos_to_brightness(float x, float z, float r, float inner_r, float outer_r,
                        float max_brightness) {
  float angle = atan2f(z, x);
  float curve = -(r - inner_r) * (r - outer_r);
  curve /= ((outer_r - inner_r) / 2.0f) * ((outer_r - inner_r) / 2.0f);
  float norm_r = (r - inner_r) / (outer_r) + 1;
  norm_r *= sinf((r - inner_r) / (outer_r - inner_r) * 2.0f * M_PI);
  return max_brightness * curve * (1.0f - norm_r) *
         (0.5f + 0.5f * (cosf(4.0f * angle + 3.0f * norm_r) + 0.6f));
}

float pos_to_brightness2(float x, float z, float r, float inner_r,
                         float outer_r, float max_brightness) {
  float angle = atan2f(z, x);
  float norm_r = (r - inner_r) / (outer_r);
  return max_brightness * norm_r;
}

float pos_to_brightness3(float x, float z, float r, float inner_r,
                         float outer_r, float max_brightness) {
  float curve = -(r - inner_r) * (r - outer_r);
  curve /= ((outer_r - inner_r) / 2.0f) * ((outer_r - inner_r) / 2.0f);
  return max_brightness * curve;
}