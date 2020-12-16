#include "websimd.h"
#include "baseline.hpp"

#define BLOCK_SIZE 2

EM_PORT_API(void) freeBuff(void* buf) {
	aligned_free(buf);
}

EM_PORT_API(double) add(double a, double b) {
  return a + b;
}

EM_PORT_API(double) addArray(double* a, int n) {
  vec_f128 vr = set_vec_f128(0.0, 0.0);

  for (int i = 0; i < n; i += BLOCK_SIZE) {
    vec_f128 va = load_vec_f128(a + i);
    vr = add_vec_f128(vr, va);
  }

  // XXX
  double *r = (double *)aligned_alloc(2 * sizeof(double), 16);
  store_vec_f128(r, vr);
  double sum = r[0] + r[1];
  aligned_free(r);
  return sum;
}
