#include "cuda_check.cuh"
#include "dim.cuh"
#include "user.cuh"

#include "iteration.hxx"
#include "timer.hxx"

#include "cuda_check.cuh"
#include "dim.cuh"
#include "user.cuh"

#include "dim.hxx"
#include "iteration.hxx"
#include "timer.hxx"

__constant__ int d_n[3];
__constant__ double d_xmin[3];
__constant__ double d_dx[3];
__constant__ double d_lambda[3];

void setDims(const int *h_n, const double *h_xmin, const double *h_dx,
             const double *h_lambda) {
  Timer &T = GetTimer(5);
  T.start();

  // Use cudaMemcpyToSymbol directly now that we are in the same file/module as
  // the definition or stick to the safer cudaGetSymbolAddress approach if
  // desired, but direct use is standard if visibility is good. We'll stick to
  // the safer approach causing the least friction, but since we are DEFINING it
  // here:

  CUDA_CHECK_OP(cudaMemcpyToSymbol(d_n, h_n, 3 * sizeof(int)));
  CUDA_CHECK_OP(cudaMemcpyToSymbol(d_xmin, h_xmin, 3 * sizeof(double)));
  CUDA_CHECK_OP(cudaMemcpyToSymbol(d_dx, h_dx, 3 * sizeof(double)));
  CUDA_CHECK_OP(cudaMemcpyToSymbol(d_lambda, h_lambda, 3 * sizeof(double)));

  cudaDeviceSynchronize();
  T.stop();
}

__global__ void iteration_kernel(double *v, const double *u, double dt,
                                 int imin, int imax, int jmin, int jmax,
                                 int kmin, int kmax) {
  int i = threadIdx.x + blockIdx.x * blockDim.x;
  int j = threadIdx.y + blockIdx.y * blockDim.y;
  int k = threadIdx.z + blockIdx.z * blockDim.z;

  if (i >= imin && i <= imax && j >= jmin && j <= jmax && k >= kmin &&
      k <= kmax) {
    int p = i + d_n[0] * (j + d_n[1] * k);

    // Indices of neighbors
    int p_im1 = p - 1;
    int p_ip1 = p + 1;
    int p_jm1 = p - d_n[0];
    int p_jp1 = p + d_n[0];
    int p_km1 = p - d_n[0] * d_n[1];
    int p_kp1 = p + d_n[0] * d_n[1];

    double lap = (u[p_im1] + u[p_ip1] - 2.0 * u[p]) * d_lambda[0] +
                 (u[p_jm1] + u[p_jp1] - 2.0 * u[p]) * d_lambda[1] +
                 (u[p_km1] + u[p_kp1] - 2.0 * u[p]) * d_lambda[2];

    // Coordinates for force
    double x = d_xmin[0] + i * d_dx[0];
    double y = d_xmin[1] + j * d_dx[1];
    double z = d_xmin[2] + k * d_dx[2];

    double f = force(x, y, z);

    v[p] = u[p] + dt * (lap + f);
  }
}

void iteration(Values &v, Values &u, double dt, int n[3], int imin, int imax,
               int jmin, int jmax, int kmin, int kmax) {
  dim3 dimBlock(8, 8, 8);
  dim3 dimGrid((n[0] + dimBlock.x - 1) / dimBlock.x,
               (n[1] + dimBlock.y - 1) / dimBlock.y,
               (n[2] + dimBlock.z - 1) / dimBlock.z);

  iteration_kernel<<<dimGrid, dimBlock>>>(v.dataGPU(), u.dataGPU(), dt, imin,
                                          imax, jmin, jmax, kmin, kmax);
  cudaDeviceSynchronize();
  CUDA_CHECK_KERNEL();
}
