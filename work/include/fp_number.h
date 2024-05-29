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
#include "fpu_instructions.h"
#include <iostream>


#ifdef RV64
  using unsignedType = unsigned long; //XLEN = 64 need 64 bit unsigned
  using signedType = long; //XLEN = 64 need 64 bit signed
#else
  using unsignedType = unsigned int; //XLEN = 32 need 32 bit unsigned
  using signedType = int; //XLEN = 32 need 32 bit signed
#endif

#ifdef EXT_D
  using loadType = unsigned long; //FLEN = 64 need 64 bit load
#else
  using loadType = unsigned int; //FLEN = 32 need 32 bit load
#endif

typedef union{
  double d;
  float f;
  struct {
    unsigned long mantissa : 52;
    unsigned int exponent : 11;
    unsigned int sign : 1;
  } d_parts;
  struct {
      unsigned int mantissa : 23;
      unsigned int exponent : 8;
      unsigned int sign : 1;
  } parts;

  uint64_t bitpattern_64;
  unsigned long u_64;
  long s_64;

  uint32_t bitpattern;
  unsigned int u;
  int s;
} FPType;


class FPNumber {
  private:
    FPType data;
    RISCV_FMT fmt;

  public:
    FPNumber();
    FPNumber(RISCV_FMT fmt);
    FPNumber( float value);
    FPNumber( unsigned int value);
    FPNumber( signed int value);
    ~FPNumber();
    FPNumber operator+();
    FPNumber operator-();
    FPNumber operator+(FPNumber num);
    FPNumber operator-(FPNumber num);
    FPNumber operator/(FPNumber num);
    FPNumber operator*(FPNumber num);


    bool operator==(FPNumber num);
    bool operator<=(FPNumber num);
    bool operator>=(FPNumber num);
    bool operator>(FPNumber num);
    bool operator<(FPNumber num);

    bool getSign();
    void setSign(bool sign);

    unsigned int getExponent();
    unsigned long getMantissa();


    //Assignment overload
    FPNumber& operator=(FPNumber num);
    FPNumber& operator=(float value);  // Overload for float
    FPNumber& operator=(unsigned int value);  // Overload for uint
    FPNumber& operator=(signed int value); // Overload for sint
    //Equal operator overload
    bool operator==(float value);  // Overload for float
    bool operator==(unsigned int value);  // Overload for uint
    bool operator==(signed int value); // Overload for sint



    operator float() const { return data.f; };
    operator unsigned int() const { return data.u; };
    operator int() const { return data.s; };

    FPNumber( double value);
    FPNumber( unsigned long value);
    FPNumber( signed long value);

    FPNumber& operator=(double value); // Overload for double
    FPNumber& operator=(unsigned long value);  // Overload for uint
    FPNumber& operator=(signed long value); // Overload for sint


    bool operator==(double value); // Overload for double
    bool operator==(unsigned long value);  // Overload for uint
    bool operator==(signed long value); // Overload for sint

    operator double() const { return data.d; };
    operator unsigned long() const { return data.u_64; };
    operator long() const { return data.s_64; };

};
