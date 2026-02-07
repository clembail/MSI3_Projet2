#include "cuda_check.cuh"
#include "dim.cuh"
#include "user.cuh"

#include "iteration.hxx"
#include "timer.hxx"

// A completer : definition du noyau

__global__ void iteration_kernel(double *v, const double *u, double dt,
                                 int imin, int imax, int jmin, int jmax,
                                 int kmin, int kmax) {
  int i = threadIdx.x + blockIdx.x * blockDim.x + imin;
  int j = threadIdx.y + blockIdx.y * blockDim.y + jmin;
  int k = threadIdx.z + blockIdx.z * blockDim.z + kmin;

  if (i <= imax && j <= jmax && k <= kmax) {
    int p = i + d_n[0] * (j + k * d_n[1]);

    int pxm = (i - 1) + d_n[0] * (j + k * d_n[1]);
    int pxp = (i + 1) + d_n[0] * (j + k * d_n[1]);

    int pym = i + d_n[0] * ((j - 1) + k * d_n[1]);
    int pyp = i + d_n[0] * ((j + 1) + k * d_n[1]);

    int pzm = i + d_n[0] * (j + (k - 1) * d_n[1]);
    int pzp = i + d_n[0] * (j + (k + 1) * d_n[1]);

    double laplacian = d_lambda[0] * (u[pxp] - 2.0 * u[p] + u[pxm]) +
                       d_lambda[1] * (u[pyp] - 2.0 * u[p] + u[pym]) +
                       d_lambda[2] * (u[pzp] - 2.0 * u[p] + u[pzm]);

    v[p] = u[p] + dt * laplacian;
  }
}

void iteration(Values &v, Values &u, double dt, int n[3], int imin, int imax,
               int jmin, int jmax, int kmin, int kmax) {
  int nx = imax - imin + 1;
  int ny = jmax - jmin + 1;
  int nz = kmax - kmin + 1;

  dim3 dimBlock(8, 8, 8);
  dim3 dimGrid((nx + dimBlock.x - 1) / dimBlock.x,
               (ny + dimBlock.y - 1) / dimBlock.y,
               (nz + dimBlock.z - 1) / dimBlock.z);

  iteration_kernel<<<dimGrid, dimBlock>>>(v.dataGPU(), u.dataGPU(), dt, imin,
                                          imax, jmin, jmax, kmin, kmax);
  CUDA_CHECK_KERNEL();
}
