#pragma once

// SI ON EST DANS LE COMPILATEUR METAL
#ifdef __METAL_VERSION__
  #include <metal_stdlib>
  using namespace metal;
  // Les fonctions 'inline' en C++ doivent être 'inline' en Metal aussi, c'est compatible.
  // Mais attention aux fonctions mathématiques (abs, sin...) qui doivent venir de metal_stdlib.
#else
  // SI ON EST EN C++ (CPU)
  #include <cmath>
  #include <algorithm>
  using namespace std;
#endif

inline double cond_ini(double x, double y, double z)
{
  x -= 0.5;
  y -= 0.5;
  z -= 0.5;
  if (x * x + y * y + z * z < 0.1)
    return 1.0;
  else
    return 0.0;
}

inline double cond_lim(double x, double y, double z)
{
  return 0.0;
}

inline double force(double x, double y, double z, double t)
{
  return sin(x - 0.5) * cos(y - 0.5) * exp(-z * z);
}
