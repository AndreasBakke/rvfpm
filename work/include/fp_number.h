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

typedef union{
  // double d;
  float f;

  // struct {
  //   unsigned long mantissa : 52;
  //   unsigned int exponent : 11;
  //   unsigned int sign : 1;
  // } d_parts;

  struct {
      unsigned int mantissa : 23;
      unsigned int exponent : 8;
      unsigned int sign : 1;
  } parts;

  uint64_t bitpattern_64;
  uint32_t bitpattern;
  unsigned long u_64;
  long s_64;
  unsigned int u;
  int s;
} FPNumber;