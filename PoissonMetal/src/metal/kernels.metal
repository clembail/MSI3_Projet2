#include <metal_stdlib>
using namespace metal;

#include "SharedStructs.h"
#include "User.h"

kernel void k_iteration(
  device const double* [[Buffer(0)]],
  device double* [[Buffer(1)]],
  constant constants& [[Buffer(2)]],
  uint [[ThreadPositionInGrid]])
{

};
