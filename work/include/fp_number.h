/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Custom floating point types. To be expanded
*/

#pragma once
#include <cmath> //Needed for NAN
#include <limits>
#include <cstdint>
#include "defines.h"

//Include convertion functions here depending on ifdefs?
#if defined(EXT_Q)

#elif defined(EXT_D)
  typedef union {
    float f;
    struct {
      unsigned int sign : 1;
      unsigned int exponent : 11;
      unsigned int mantissa : 52;
    } parts;
  } FPNumber;
#else
  typedef union {
    float f;
    struct {
      unsigned int mantissa : 23;
      unsigned int exponent : 8;
      unsigned int sign : 1;
    } parts;
    uint32_t bitpattern; //bitpattern. NOTE: no interpretation of value should be done.
    unsigned int u; //unsigned int representation
    int s; //signed int representation
  } FPNumber;
#endif
