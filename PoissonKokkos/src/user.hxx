
#ifndef __USER_CXX__
#define __USER_CXX__

#include <cmath>
#include "type.hxx"
#include <Kokkos_Core.hpp>

/* Remplacer par des "fonctions inline kokkos" */

KOKKOS_INLINE_FUNCTION
REAL_TYPE cond_ini(REAL_TYPE x, REAL_TYPE y, REAL_TYPE z)
{
  x -= 0.5;
  y -= 0.5;
  z -= 0.5;
  if (x * x + y * y + z * z < 0.1)
    return 1.0;
  else
    return 0.0;
}

KOKKOS_INLINE_FUNCTION
REAL_TYPE cond_lim(REAL_TYPE x, REAL_TYPE y, REAL_TYPE z)
{
  return 0.0;
}

KOKKOS_INLINE_FUNCTION
REAL_TYPE force(REAL_TYPE x, REAL_TYPE y, REAL_TYPE z, REAL_TYPE t)
{
  return sin(x - 0.5) * cos(y - 0.5) * exp(-z * z);
}


#endif
