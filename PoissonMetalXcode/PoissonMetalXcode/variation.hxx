#include "values.hxx"

void allocVariationData(float *& diff, int n,
  float *& diffPartial, int nPartial);

void freeVariationData(float *& diff,
  float *& diffPartial);

float variation
 (const Values & u, const Values & v,
  float *& diff, float *&diffPartial, int n);
