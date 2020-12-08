#include "websimd.h"
#include "softfloat.hpp"

EM_PORT_API(double) add(double a, double b) {
  softdouble r = softdouble(a) + softdouble(b);
  return double(r);
}
