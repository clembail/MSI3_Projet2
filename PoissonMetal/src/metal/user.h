#pragma once

#ifdef __METAL_VERSION__
  #include <metal_stdlib>
  using namespace metal;
#else
  #include <cmath>
  #include <algorithm>
  using namespace std;
#endif

inline float cond_ini(float x, float y, float z)
{
  x -= 0.5f;
  y -= 0.5f;
  z -= 0.5f;
  if (x * x + y * y + z * z < 0.1f)
    return 1.0f;
  else
    return 0.0f;
}

inline float cond_lim(float x, float y, float z)
{
  return 0.0f;
}

inline float force(float x, float y, float z, float t)
{
  return sin(x - 0.5f) * cos(y - 0.5f) * exp(-z * z);
}
