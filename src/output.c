/*!
 * output.c - output for libsatoshi
 * Copyright (c) 2021, Christopher Jeffrey (MIT License).
 * https://github.com/chjj/libsatoshi
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <satoshi/consensus.h>
#include <satoshi/script.h>
#include <satoshi/tx.h>
#include <torsion/hash.h>
#include "impl.h"
#include "internal.h"

/*
 * Output
 */

DEFINE_SERIALIZABLE_OBJECT(btc_output, SCOPE_EXTERN)

void
btc_output_init(btc_output_t *z) {
  z->value = 0;
  btc_script_init(&z->script);
}

void
btc_output_clear(btc_output_t *z) {
  btc_script_clear(&z->script);
}

void
btc_output_copy(btc_output_t *z, const btc_output_t *x) {
  btc_script_copy(&z->script, &x->script);
}

size_t
btc_output_size(const btc_output_t *x) {
  size_t size = 0;

  size += 8;
  size += btc_script_size(&x->script);

  return size;
}

uint8_t *
btc_output_write(uint8_t *zp, const btc_output_t *x) {
  zp = btc_int64_write(zp, x->value);
  zp = btc_script_write(zp, &x->script);
  return zp;
}

int
btc_output_read(btc_output_t *z, const uint8_t **xp, size_t *xn) {
  if (!btc_int64_read(&z->value, xp, xn))
    return 0;

  if (!btc_script_read(&z->script, xp, xn))
    return 0;

  return 1;
}

void
btc_output_update(hash256_t *ctx, const btc_output_t *x) {
  btc_int64_update(ctx, x->value);
  btc_script_update(ctx, &x->script);
}

/* XXX */
#define btc_get_min_fee(x, y) 1

int64_t
btc_output_dust_threshold(const btc_output_t *x, int64_t rate) {
  int scale = BTC_WITNESS_SCALE_FACTOR;
  size_t size;

  if (btc_script_is_unspendable(&x->script))
    return 0;

  size = btc_output_size(x);

  if (btc_script_is_program(&x->script)) {
    /* 75% segwit discount applied to script size. */
    size += 32 + 4 + 1 + (107 / scale) + 4;
  } else {
    size += 32 + 4 + 1 + 107 + 4;
  }

  return 3 * btc_get_min_fee(size, rate);
}

int64_t
btc_output_is_dust(const btc_output_t *x, int64_t rate) {
  return x->value < btc_output_dust_threshold(x, rate);
}
