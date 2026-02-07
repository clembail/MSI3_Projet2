#include <metal_stdlib>
using namespace metal;

#include "SharedStructs.h"
#include "User.h"



kernel void k_iteration(
  device const float* u         [[buffer(0)]],
  device float* v               [[buffer(1)]],
  constant constants& cst       [[buffer(2)]],
  uint3 id                      [[thread_position_in_grid]])
{
  uint i = id.x, j = id.y, k = id.z;
  uint nx = cst.d_n[0], ny = cst.d_n[1], nz = cst.d_n[2];
  if (i == 0 || i >= nx -1 ||
      j == 0 || j >= ny -1 ||
      k == 0 || k >= nz -1){ // bords
    return;
  }
  int index = i + j*nx + k*nx*ny;

  int index_im1 = index - 1;
  int index_ip1 = index + 1;
  int index_jm1 = index - nx;
  int index_jp1 = index + nx;
  int index_km1 = index - nx * ny;
  int index_kp1 = index + nx * ny;

  float lap = (u[index_im1] + u[index_ip1] - 2.0 * u[index]) * cst.d_lambda[0] +
               (u[index_jm1] + u[index_jp1] - 2.0 * u[index]) * cst.d_lambda[1] +
               (u[index_km1] + u[index_kp1] - 2.0 * u[index]) * cst.d_lambda[2];

  // Coordinates for force
  float x = cst.d_xmin[0] + i * cst.d_dx[0];
  float y = cst.d_xmin[1] + j * cst.d_dx[1];
  float z = cst.d_xmin[2] + k * cst.d_dx[2];

  float f = force(x, y, z, 0);

  v[index] = u[index] + cst.d_dt * (lap + f);
}



kernel void k_init(
  device float* u         [[buffer(0)]],
  constant constants& cst [[buffer(2)]],
  uint3 id                [[thread_position_in_grid]])
{
  uint i = id.x, j = id.y, k = id.z;
  uint nx = cst.d_n[0], ny = cst.d_n[1], nz = cst.d_n[2];
  int index;

  if (i>=nx || j>=ny || k>=nz){
    return;
  }

  index = i + j*nx + k*nx*ny;
  float x = cst.d_xmin[0] + i * cst.d_dx[0];
  float y = cst.d_xmin[1] + j * cst.d_dx[1];
  float z = cst.d_xmin[2] + k * cst.d_dx[2];

  u[index] = cond_ini(x, y, z);
}


kernel void k_boundaries(
  device float* u         [[buffer(0)]],
  constant constants& cst [[buffer(2)]],
  uint3 id                [[thread_position_in_grid]])
{
  uint i = id.x, j = id.y, k = id.z;
  uint nx = cst.d_n[0], ny = cst.d_n[1], nz = cst.d_n[2];
  int index;

  if (i>=nx || j>=ny || k>=nz){
    return;
  }

  if (i==0 || j==0 || k==0 || i==nx-1 || j==ny-1 || k==nz-1){
    index = i + j*nx + k*nx*ny;
    float x = cst.d_xmin[0] + i * cst.d_dx[0];
    float y = cst.d_xmin[1] + j * cst.d_dx[1];
    float z = cst.d_xmin[2] + k * cst.d_dx[2];

    u[index] = cond_lim(x, y, z);
  }
}



kernel void k_difference(
  device const float* u [[buffer(0)]],
  device const float* v [[buffer(1)]],
  device float* diff    [[buffer(2)]],
  constant int* n       [[buffer(3)]],
  uint id               [[thread_position_in_grid]])
{
  if (id >= *n) return;

  diff[id] = abs(u[id] - v[id]);
}




kernel void k_reduce(
  device const float* input      [[buffer(0)]],
  device float* partialSums      [[buffer(1)]],
  constant int* n                [[buffer(2)]],
  uint gid                       [[thread_position_in_grid]],
  uint tid                       [[thread_position_in_threadgroup]],
  uint group_id                  [[threadgroup_position_in_grid]],
  uint group_size                [[threads_per_threadgroup]],
  threadgroup float* sdata       [[threadgroup(0)]]
)
{
  (gid < *n) ? sdata[tid] = input[gid] : sdata[tid] = 0.0;

  threadgroup_barrier(mem_flags::mem_threadgroup);

  for (uint s = group_size/2; s > 0; s >>= 1){
    if (tid < s){
      sdata[tid] += sdata[tid+s];
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);
  }

  if (tid == 0) partialSums[group_id] = sdata[0];
}
