#pragma once

#include "websimd.h"
#include "softfloat.hpp"
#include <wasm_simd128.h>

uint_fast8_t globalDetectTininess = tininess_afterRounding;
uint_fast8_t globalRoundingMode = round_min;

// Type used to pass 64-bit floating-point arguments and results
// to/from functions
typedef softdouble float64_t;

#define signF64UI(a) (((uint64_t)(a) >> 63) != 0)

float64_t f64_add_simd(float64_t a, float64_t b);
static float64_t softfloat_addMagsF64(uint_fast64_t, uint_fast64_t, bool);
static float64_t softfloat_subMagsF64(uint_fast64_t, uint_fast64_t, bool);

float64_t f64_add_simd(float64_t a, float64_t b) {
  uint_fast64_t uiA;
  bool signA;
  uint_fast64_t uiB;
  bool signB;

  uiA = a.v;
  signA = signF64UI(uiA);
  uiB = b.v;
  signB = signF64UI(uiB);
  if (signA == signB) {
    return softfloat_addMagsF64(uiA, uiB, signA);
  } else {
    return softfloat_subMagsF64(uiA, uiB, signA);
  }
}

static float64_t softfloat_addMagsF64(uint_fast64_t uiA, uint_fast64_t uiB,
                                      bool signZ) {
  int_fast16_t expA;
  uint_fast64_t sigA;
  int_fast16_t expB;
  uint_fast64_t sigB;
  int_fast16_t expDiff;
  uint_fast64_t uiZ;
  int_fast16_t expZ;
  uint_fast64_t sigZ;

  /*------------------------------------------------------------------------
   *------------------------------------------------------------------------*/
  expA = expF64UI(uiA);
  sigA = fracF64UI(uiA);
  expB = expF64UI(uiB);
  sigB = fracF64UI(uiB);
  /*------------------------------------------------------------------------
   *------------------------------------------------------------------------*/
  expDiff = expA - expB;
  if (!expDiff) {
    /*--------------------------------------------------------------------
     *--------------------------------------------------------------------*/
    if (!expA) {
      uiZ = uiA + sigB;
      goto uiZ;
    }
    if (expA == 0x7FF) {
      if (sigA | sigB) goto propagateNaN;
      uiZ = uiA;
      goto uiZ;
    }
    expZ = expA;
    sigZ = UINT64_C(0x0020000000000000) + sigA + sigB;
    sigZ <<= 9;
  } else {
    /*--------------------------------------------------------------------
     *--------------------------------------------------------------------*/
    sigA <<= 9;
    sigB <<= 9;
    if (expDiff < 0) {
      if (expB == 0x7FF) {
        if (sigB) goto propagateNaN;
        uiZ = packToF64UI(signZ, 0x7FF, 0);
        goto uiZ;
      }
      expZ = expB;
      if (expA) {
        sigA += UINT64_C(0x2000000000000000);
      } else {
        sigA <<= 1;
      }
      sigA = softfloat_shiftRightJam64(sigA, -expDiff);
    } else {
      if (expA == 0x7FF) {
        if (sigA) goto propagateNaN;
        uiZ = uiA;
        goto uiZ;
      }
      expZ = expA;
      if (expB) {
        sigB += UINT64_C(0x2000000000000000);
      } else {
        sigB <<= 1;
      }
      sigB = softfloat_shiftRightJam64(sigB, expDiff);
    }
    sigZ = UINT64_C(0x2000000000000000) + sigA + sigB;
    if (sigZ < UINT64_C(0x4000000000000000)) {
      --expZ;
      sigZ <<= 1;
    }
  }
  return softfloat_roundPackToF64(signZ, expZ, sigZ);
  /*------------------------------------------------------------------------
   *------------------------------------------------------------------------*/
propagateNaN:
  uiZ = softfloat_propagateNaNF64UI(uiA, uiB);
uiZ:
  return float64_t::fromRaw(uiZ);
}

static float64_t softfloat_subMagsF64(uint_fast64_t uiA, uint_fast64_t uiB,
                                      bool signZ) {
  int_fast16_t expA;
  uint_fast64_t sigA;
  int_fast16_t expB;
  uint_fast64_t sigB;
  int_fast16_t expDiff;
  uint_fast64_t uiZ;
  int_fast64_t sigDiff;
  int_fast8_t shiftDist;
  int_fast16_t expZ;
  uint_fast64_t sigZ;

  /*------------------------------------------------------------------------
   *------------------------------------------------------------------------*/
  expA = expF64UI(uiA);
  sigA = fracF64UI(uiA);
  expB = expF64UI(uiB);
  sigB = fracF64UI(uiB);
  /*------------------------------------------------------------------------
   *------------------------------------------------------------------------*/
  expDiff = expA - expB;
  if (!expDiff) {
    /*--------------------------------------------------------------------
     *--------------------------------------------------------------------*/
    if (expA == 0x7FF) {
      if (sigA | sigB) goto propagateNaN;
      raiseFlags(flag_invalid);
      uiZ = defaultNaNF64UI;
      goto uiZ;
    }
    sigDiff = sigA - sigB;
    if (!sigDiff) {
      uiZ = packToF64UI((globalRoundingMode == round_min), 0, 0);
      goto uiZ;
    }
    if (expA) --expA;
    if (sigDiff < 0) {
      signZ = !signZ;
      sigDiff = -sigDiff;
    }
    shiftDist = softfloat_countLeadingZeros64(sigDiff) - 11;
    expZ = expA - shiftDist;
    if (expZ < 0) {
      shiftDist = (int_fast8_t)expA;  // fixed type cast
      expZ = 0;
    }
    uiZ = packToF64UI(signZ, expZ, sigDiff << shiftDist);
    goto uiZ;
  } else {
    /*--------------------------------------------------------------------
     *--------------------------------------------------------------------*/
    sigA <<= 10;
    sigB <<= 10;
    if (expDiff < 0) {
      /*----------------------------------------------------------------
       *----------------------------------------------------------------*/
      signZ = !signZ;
      if (expB == 0x7FF) {
        if (sigB) goto propagateNaN;
        uiZ = packToF64UI(signZ, 0x7FF, 0);
        goto uiZ;
      }
      sigA += expA ? UINT64_C(0x4000000000000000) : sigA;
      sigA = softfloat_shiftRightJam64(sigA, -expDiff);
      sigB |= UINT64_C(0x4000000000000000);
      expZ = expB;
      sigZ = sigB - sigA;
    } else {
      /*----------------------------------------------------------------
       *----------------------------------------------------------------*/
      if (expA == 0x7FF) {
        if (sigA) goto propagateNaN;
        uiZ = uiA;
        goto uiZ;
      }
      sigB += expB ? UINT64_C(0x4000000000000000) : sigB;
      sigB = softfloat_shiftRightJam64(sigB, expDiff);
      sigA |= UINT64_C(0x4000000000000000);
      expZ = expA;
      sigZ = sigA - sigB;
    }
    return softfloat_normRoundPackToF64(signZ, expZ - 1, sigZ);
  }
  /*------------------------------------------------------------------------
   *------------------------------------------------------------------------*/
propagateNaN:
  uiZ = softfloat_propagateNaNF64UI(uiA, uiB);
uiZ:
  return float64_t::fromRaw(uiZ);
}

