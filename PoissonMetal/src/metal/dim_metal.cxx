#include "dim.hxx"
#include "SharedStructs.h"
#include "timer.hxx"

static constants _hostConstants;

void setDims(const int *h_n,
             const float *h_xmin,
             const float *h_dx,
             const float *h_lambda)
{
  Timer & T = GetTimer(5); T.start();
  for (int i = 0; i < 3; i++){
    _hostConstants.d_n[i] = h_n[i];
    _hostConstants.d_xmin[i] = h_xmin[i];
    _hostConstants.d_dx[i] = h_dx[i];
    _hostConstants.d_lambda[i] = h_lambda[i];
  }
  T.stop();

}

constants& getConstants(){
  return _hostConstants;
}
