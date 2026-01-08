
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

int initialize_photon(photon& p, blackhole& bh) {
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

  return 0;
}

int relativistic_simulation(photon& p, blackhole& bh, float dlambda) {
  float r2 = p.r * p.r;
  float r3 = r2 * p.r;

  float d2r = (bh.sradius * p.L * p.L) / (r2 * r2) - (bh.sradius / (2 * r2));

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

  return 0;
}

int non_relativistic_simulation(photon& p) {
  float old_r = p.r;
  p.x += p.vx;
  p.y += p.vy;
  p.z += p.vz;
  p.r = sqrtf(p.x * p.x + p.y * p.y + p.z * p.z);
  p.dr = p.r - old_r;

  return 0;
}

vec3 photon_direction(int x, int y, float theta, float phi, float fov,
                      int width, int height) {
  float horizontal_id =
      (x - (width / 2)) +
      0.5f;  // horizontal and vertical refer to pixel locations in output image
  float vertical_id = (y - (height / 2)) + 0.5f;

  float horizontal_range = tanf(fov / 2.0f);
  float vertical_range = tanf(fov / 2.0f);

  float horizontal_offset = horizontal_range / width;
  float vertical_offset = vertical_range / height;

  vec3 target = {1.0f, vertical_id * vertical_offset,
                 horizontal_id * horizontal_offset};

  float length =
      sqrtf(target.x * target.x + target.y * target.y + target.z * target.z);
  vec3 normalized_target = {target.x / length, target.y / length,
                            target.z / length};

  // manual implementation of rotation matrix to match the camera direction
  // implemented with two distinct rotations around the z-

  vec3 first_rotation = {
      cosf(phi) * normalized_target.x - sinf(phi) * normalized_target.y,
      sinf(phi) * normalized_target.x + cosf(phi) * normalized_target.y,
      normalized_target.z};

  vec3 second_rotation = {
      first_rotation.x,
      cosf(theta) * first_rotation.y - sinf(theta) * first_rotation.z,
      sinf(theta) * first_rotation.y + cosf(theta) * first_rotation.z};

  return second_rotation;
}