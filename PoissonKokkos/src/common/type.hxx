#ifndef REAL_TYPE 
  #ifdef FLOAT_TYPE
    #define REAL_TYPE float
  #else
    #define REAL_TYPE double
  #endif
#endif