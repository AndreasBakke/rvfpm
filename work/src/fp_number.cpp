/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Functions for FP_number class.
*/
#include "fp_number.h"

FPNumber::FPNumber() {

}

FPNumber::~FPNumber() {
}
FPNumber::FPNumber(RISCV_FMT fmt){
  this->fmt = fmt;
};

FPNumber::FPNumber(float value) {
  data.f = value;
  this->fmt = RISCV_FMT::S;
}

FPNumber::FPNumber(unsigned int value){
  data.u = value;
};

FPNumber::FPNumber(signed int value){
  data.s = value;
};

FPNumber FPNumber::operator+(){
  if(this->fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: +" << std::endl;
      abort();
    #endif
    return FPNumber(this->data.d);
  }
  return FPNumber(this->data.f);
};
FPNumber FPNumber::operator-(){
  if(this->fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: -" << std::endl;
      abort();
    #endif
    return FPNumber(-this->data.d);
  }
  return FPNumber(-this->data.f);
};

FPNumber FPNumber::operator+(FPNumber num){
  if(num.fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: + num" << std::endl;
      abort();
    #endif
    return FPNumber(this->data.d + num.data.d);
  }
  return FPNumber(this->data.f + num.data.f);
};

FPNumber FPNumber::operator-(FPNumber num){
  if(num.fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: - num" << std::endl;
      abort();
    #endif
    return FPNumber(this->data.d - num.data.d);
  }
  return FPNumber(this->data.f - num.data.f);
};

FPNumber FPNumber::operator/(FPNumber num){
  if(num.fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: /num" << std::endl;
      abort();
    #endif
    return FPNumber(this->data.d / num.data.d);
  }
  return FPNumber(this->data.f / num.data.f);
};

FPNumber FPNumber::operator*(FPNumber num){
  if(num.fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: *num" << std::endl;
      abort();
    #endif
    return FPNumber(this->data.d * num.data.d);
  }
  return FPNumber(this->data.f * num.data.f);
};

bool FPNumber::operator==(FPNumber num){
  if(num.fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: ==" << std::endl;
      abort();
    #endif
    return this->data.d == num.data.d;
  }
  return this->data.f == num.data.f;
};

bool FPNumber::operator<=(FPNumber num){
  if(num.fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: <=" << std::endl;
      abort();
    #endif
    return this->data.d <= num.data.d;
  }
  return this->data.f <= num.data.f;
};

bool FPNumber::operator>=(FPNumber num){
  if(num.fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: >=" << std::endl;
      abort();
    #endif
    return this->data.d >= num.data.d;
  }
  return this->data.f >= num.data.f;
};

bool FPNumber::operator>(FPNumber num){
  if(num.fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: >" << std::endl;
      abort();
    #endif
    return this->data.d > num.data.d;
  }
  return this->data.f > num.data.f;
};

bool FPNumber::operator<(FPNumber num){
  if(num.fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: <" << std::endl;
      abort();
    #endif
    return this->data.d < num.data.d;
  }
  return this->data.f < num.data.f;
};

bool FPNumber::getSign(){
  if(this->fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: getSign()" << std::endl;
      abort();
    #endif
    return this->data.d_parts.sign;
  }
  return this->data.parts.sign;
};

void FPNumber::setSign(bool sign){
  if(this->fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: setSign()" << std::endl;
      abort();
    #endif
    this->data.d_parts.sign = sign;
  } else {
    this->data.parts.sign = sign;
  }
};

unsigned int FPNumber::getExponent(){
  if(this->fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: getExponent()" << std::endl;
      abort();
    #endif
    return this->data.d_parts.exponent;
  }
  return this->data.parts.exponent;
}

unsigned long FPNumber::getMantissa(){
  if(this->fmt == RISCV_FMT::D){
    #ifndef EXT_D
      std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: getMantissa()" << std::endl;
      abort();
    #endif
    return this->data.d_parts.mantissa;
  }
  return this->data.parts.mantissa;
}

FPNumber& FPNumber::operator=(FPNumber num){
  this->data = num.data;
  return *this;
};

FPNumber& FPNumber::operator=(float value) {
  this->fmt = RISCV_FMT::S;
  this->data.f = value;
  return *this;
}

FPNumber& FPNumber::operator=(unsigned int value){
  this->data.u = value;
  return *this;
};

FPNumber& FPNumber::operator=(int value){
  this->data.s = value;
  return *this;
};

bool FPNumber::operator==(float value) {
  this->fmt = RISCV_FMT::S;
  return this->data.f == value;
}

bool FPNumber::operator==(unsigned int value){
  return this->data.u == value;
};

bool FPNumber::operator==(int value){
  return this->data.s == value;
};

FPNumber::FPNumber(signed long value){
  data.s_64 = value;
};

FPNumber::FPNumber(unsigned long value){
  data.u_64 = value;
};

FPNumber::FPNumber(double value) {
  #ifndef EXT_D
    std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: FPNumber(double)" << std::endl;
  #endif
  this->fmt = RISCV_FMT::D;
  data.d = value;
}

FPNumber& FPNumber::operator=(double value) {
  #ifndef EXT_D
    std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: =(double)" << std::endl;
  #endif
  this->data.d = value;
  this->fmt = RISCV_FMT::D;
  return *this;
}

FPNumber& FPNumber::operator=(unsigned long value){
  this->data.u_64 = value;
  return *this;
};

FPNumber& FPNumber::operator=(long value){
  this->data.s_64 = value;
  return *this;
};

bool FPNumber::operator==(double value) {
  #ifndef EXT_D
    std::cerr << "Attempting to use Double format without the D-extension. Set EXT_D in config to enable. Operator: ==(double)" << std::endl;
  #endif
  return this->data.d == value;
}

bool FPNumber::operator==(unsigned long value){
  return this->data.u_64 == value;
};

bool FPNumber::operator==(long value){
  return this->data.s_64 == value;
};