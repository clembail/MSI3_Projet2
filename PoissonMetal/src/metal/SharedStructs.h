#pragma once
#include <simd/simd.h>

struct constants{
  int d_n[3];
  double d_xmin[3];
  double d_dx[3];
  double d_lambda[3];
  double d_dt;
};

#ifdef __cplusplus

// Forward declaration pour ne pas inclure tout Metal.hpp ici
namespace MTL { class Buffer; }

// Prototype de ta fonction helper (N'oublie pas le ;)
MTL::Buffer* getMetalBuffer(void* ptr);

#endif
