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
// #include "half-2.0.0/include/half.hpp"


//TODO: Handle custom formats like bfloat16. Might need conversion functions (or just do operations in F-extension and convert to bfloat16 for Store/Load and results)

#if defined(EXT_Q) //Quad precision - does not work
  typedef __int128 int128_t;
  typedef unsigned __int128 uint128_t;
  using unsignedType = uint128_t;
  using signedType = int128_t;
  using floatType = long double;
  typedef union {
    floatType f;
    struct {
      unsignedType mantissa : 112;
      unsigned int exponent : 15;
      unsigned int sign : 1;

    } parts;
    uint128_t bitpattern; //bitpattern. NOTE: no interpretation of value should be done.
    unsignedType u; //unsigned int representation
    signedType s; //signed int representation
  } FPNumber;

#elif defined(EXT_D) //Double precision
  using unsignedType = unsigned long;
  using signedType = long;
  using floatType = double;

  typedef union {
    floatType f;
    struct {
      unsignedType mantissa : 52;
      unsigned int exponent : 11;
      unsigned int sign : 1;
    } parts;
    uint64_t bitpattern; //bitpattern. NOTE: no interpretation of value should be done.
    unsignedType u; //unsigned int representation
    signedType s; //signed int representation
  } FPNumber;

#elif defined(EXT_H) //Half precision - Does not work
  using unsignedType = unsigned short;
  using signedType = short;
  // using half_float::half;
  using floatType = float;
  typedef union {
    floatType f;
    struct {
      unsignedType mantissa : 10;
      unsigned int exponent : 5;
      unsigned int sign : 1;
    } parts;
    uint16_t bitpattern; //bitpattern. NOTE: no interpretation of value should be done.
    unsignedType u; //unsigned int representation
    signedType s; //signed int representation
  } FPNumber;

#else //Use Single-precision as standard
  using unsignedType = unsigned int;
  using signedType = int;
  using floatType = float;
  typedef union {
    float f;
    struct {
      unsignedType mantissa : 23;
      unsigned int exponent : 8;
      unsigned int sign : 1;
    } parts;
    uint32_t bitpattern; //bitpattern. NOTE: no interpretation of value should be done.
    unsignedType u; //unsigned int representation
    signedType s; //signed int representation
  } FPNumber;
#endif
