#include "dim.hxx"
#include "SharedStructs.h"
#include "timer.hxx"

static constants _hostConstants;

// void symbol()
// {
//   printf("symbol : dx = %f %f %f\n",
//     _hostConstants.d_dx[0],
//     _hostConstants.d_dx[1],
//     _hostConstants.d_dx[2]);
//   printf("symbol : n  = %d %d %d\n",
//     _hostConstants.d_n[0],
//     _hostConstants.d_n[1],
//     _hostConstants.d_n[2]);
// }

void setDims(const int *h_n,
             const float *h_xmin,
             const float *h_dx,
             const float *h_lambda)
{
  // Timer & T = GetTimer(5); T.start();
  for (int i = 0; i < 3; i++){
    _hostConstants.d_n[i] = h_n[i];
    _hostConstants.d_xmin[i] = h_xmin[i];
    _hostConstants.d_dx[i] = h_dx[i];
    _hostConstants.d_lambda[i] = h_lambda[i];
  }
  // T.stop();

  // symbol<<<1,1>>>();
}

constants& getConstants(){
  return _hostConstants;
}
