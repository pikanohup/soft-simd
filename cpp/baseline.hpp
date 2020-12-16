#pragma once

#include "endian.h"

#include <cstdint>
#include <cstdlib>

typedef union suf64 {
  int64_t i;
  uint64_t u;
  double f;
} suf64;

typedef union {
	uint64_t u64[2];
	uint32_t u32[4];
	uint16_t u16[8];
	uint8_t u8[16];
} vec_i128;

typedef union {
	struct {
		double lo;
		double hi;
	};
	vec_i128 i;
} vec_f128;

#define aligned_alloc(a, b) malloc(a)
#define aligned_free(a) free(a)

vec_f128 set_vec_f128(double x1, double x0) {
	suf64 sx1, sx0;
	sx1.f = x1;
	sx0.f = x0;
	vec_f128 x;
	x.i.u64[0] = sx0.u;
	x.i.u64[1] = sx1.u;
	return x;
}

vec_f128 load_vec_f128(const double* pd) {
	vec_f128 x;
	x.i.u64[0] = load64(pd + 0);
	x.i.u64[1] = load64(pd + 1);
	return x;
}

void store_vec_f128(double* mem_addr, vec_f128 a) {
	store64(mem_addr + 0, a.i.u64[0]);
	store64(mem_addr + 1, a.i.u64[1]);
}

vec_f128 add_vec_f128(vec_f128 a, vec_f128 b) {
	vec_f128 x;
	x.lo = a.lo + b.lo;
	x.hi = a.hi + b.hi;
	return x;
}
