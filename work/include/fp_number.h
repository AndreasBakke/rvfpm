/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Custom floating point types. To be expanded
*/

#pragma once
#include <cmath> //Needed for NAN
#include <limits>
#include <cstdint>
#include "config.h"


#ifdef RV64
  using unsignedType = unsigned long;
  using signedType = long;
#else
  using unsignedType = unsigned int;
  using signedType = int;
#endif

#ifdef EXT_D
  using floatType = double;
  using loadType = unsigned long;
#else
  using floatType = float;
  using loadType = unsigned int;
#endif

typedef union{
  #ifdef EXT_D
    double d;
  #endif
  float f;

  #ifdef EXT_D
    struct {
      unsigned long mantissa : 52;
      unsigned int exponent : 11;
      unsigned int sign : 1;
    } d_parts;
  #endif
  struct {
      unsigned int mantissa : 23;
      unsigned int exponent : 8;
      unsigned int sign : 1;
  } parts;

  #ifdef RV64
    uint64_t bitpattern_64;
    unsigned long u_64;
    long s_64;
  #endif
  uint32_t bitpattern;
  unsigned int u;
  int s;
} FPNumber;