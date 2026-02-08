#pragma once

#include "metal/SharedStructs.h"

void setDims(const int *n,
             const float *xmin,
             const float *dx,
             const float *lambda);

 constants& getConstants();
