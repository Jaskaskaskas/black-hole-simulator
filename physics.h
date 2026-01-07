#include <deque>
#include <vector>

struct vec3 {
  float x, y, z;
};
struct trace {
  std::deque<std::pair<float, float>> head;
  size_t max_length = 255;
};
struct photon {
  int idx, idy;
  float x, y, z;  // The global 3D position
  float vx, vy, vz;
  float r, phi;
  float dr, dphi;
  float L, E;
  float b;
  float dt;
  float angle;
  float brightness = 0.0f;
  bool active;
  vec3 u1, u2;
  trace t;
};

struct accretiondisk {
  float inner_r, outer_r;
  float brightness;
  float thickness;
};

struct photons {
  std::vector<photon> list;
};

struct blackhole {
  float x, y;
  float sradius;
};

float pos_to_brightness(float x, float z, float r, float inner_r, float outer_r,
                        float max_brightness);

float pos_to_brightness2(float x, float z, float r, float inner_r,
                         float outer_r, float max_brightness);

float pos_to_brightness3(float x, float z, float r, float inner_r,
                         float outer_r, float max_brightness);

int initialize_photon(photon& p, blackhole& bh);

int relativistic_simulation(photon& p, blackhole& bh, float dlambda);

int non_relativistic_simulation(photon& p);