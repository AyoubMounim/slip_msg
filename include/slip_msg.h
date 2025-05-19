
#pragma once

#include <stdint.h>

#ifndef SLIP_PKG_MAX_SIZE
#define SLIP_PKG_MAX_SIZE 512
#endif

#if SLIP_PKG_MAX_SIZE > 0xFFFF
#error "slip packet too big (max size is 0xFFFF bytes)"
#endif

struct slip_msg_intrf {
  int32_t (*read)(void *ctx, uint16_t size, uint8_t buffer[size]);
  int32_t (*write)(void *ctx, uint16_t size, uint8_t const buffer[size]);
  int8_t (*deinit)(void *ctx);
};

struct slip_msg {
  struct slip_msg_intrf *intrf;
  void *ctx;
};

enum slip_err {
  SLIP_ERR_OK,
  SLIP_ERR_WRITE_FAIL,
  SLIP_ERR_PKG_TOO_BIG,
  SLIP_ERR_PKG_TRUNCATED,
  SLIP_ERR_PKG_INVALID_START,
  SLIP_ERR_PKG_INVALID_ESC,
  SLIP_ERR_READ_FAIL,
  SLIP_ERR_DEINIT_FAIL,
};

/* ============================================ Public functions declaration */

enum slip_err slip_msg_read(struct slip_msg const *self, uint16_t *size,
                            uint8_t buffer[*size]);

enum slip_err slip_msg_write(struct slip_msg const *self, uint16_t *size,
                             uint8_t const buffer[*size]);

enum slip_err slip_msg_deinit(struct slip_msg *self);
