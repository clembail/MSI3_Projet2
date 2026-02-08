#pragma once

// Structures partagées entre C++ et Metal
struct constants {
  float d_xmin[3];
  float d_dx[3];
  float d_lambda[3];
  float d_dt;
  float d_t;
  int d_n[3];
};


#ifndef __METAL_VERSION__

#include <Metal/Metal.hpp>

MTL::Buffer* getMetalBuffer(void* ptr);


#endif
