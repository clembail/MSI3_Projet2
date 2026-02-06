#include "dim.cuh"

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
