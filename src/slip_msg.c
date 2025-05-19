
#include "slip_msg.h"

#define END 0300
#define ESC_END 0334

#define ESC 0333
#define ESC_ESC 0335

/* ============================================= Public functions definition */

enum slip_err slip_msg_read(struct slip_msg const *self, uint16_t *size,
                            uint8_t buffer[*size]) {
  enum slip_err err = SLIP_ERR_OK;
  uint8_t truncated = 1;
  uint16_t bytes_read = 0;
  if (*size > SLIP_PKG_MAX_SIZE) {
    *size = SLIP_PKG_MAX_SIZE;
  }
  while (bytes_read < *size) {
    int32_t read =
        self->intrf->read(self->ctx, sizeof(uint8_t), buffer + bytes_read);
    if (read <= 0) {
      err = SLIP_ERR_READ_FAIL;
      goto exit;
    }
    switch (buffer[bytes_read]) {
    case END:
      if (bytes_read == 0) {
        continue;
      }
      truncated = 0;
      goto exit;
    case ESC:
      read = self->intrf->read(self->ctx, sizeof(uint8_t), buffer + bytes_read);
      if (read < 0) {
        err = SLIP_ERR_READ_FAIL;
        goto exit;
      }
      switch (buffer[bytes_read]) {
      case ESC_END:
        buffer[bytes_read] = END;
        break;
      case ESC_ESC:
        buffer[bytes_read] = ESC;
        break;
      default:
        err = SLIP_ERR_PKG_INVALID_ESC;
        goto exit;
      }
      break;
    default:
      break;
    }
    bytes_read++;
  }
  if (truncated) {
    err = SLIP_ERR_PKG_TRUNCATED;
  }
exit:
  *size = bytes_read;
  return err;
}

enum slip_err slip_msg_write(struct slip_msg const *self, uint16_t *size,
                             uint8_t const buffer[*size]) {
  enum slip_err err = SLIP_ERR_OK;
  uint16_t bytes_written = 0;
  if (*size > SLIP_PKG_MAX_SIZE) {
    err = SLIP_ERR_PKG_TOO_BIG;
    goto exit;
  }
  static uint8_t const end_char = END;
  static uint8_t const esc_end_seq[] = {ESC, ESC_END};
  static uint8_t const esc_esc_seq[] = {ESC, ESC_ESC};
  int32_t wrote = self->intrf->write(self->ctx, sizeof(end_char), &end_char);
  if (wrote < 0) {
    err = SLIP_ERR_WRITE_FAIL;
    goto exit;
  }
  while (bytes_written < *size) {
    switch (buffer[bytes_written]) {
    case END:
      wrote = self->intrf->write(self->ctx, sizeof(esc_end_seq), esc_end_seq);
      break;
    case ESC:
      wrote = self->intrf->write(self->ctx, sizeof(esc_esc_seq), esc_esc_seq);
      break;
    default:
      wrote = self->intrf->write(self->ctx, sizeof(uint8_t),
                                 buffer + bytes_written);
    }
    if (wrote < 0) {
      err = SLIP_ERR_WRITE_FAIL;
      goto exit;
    }
    bytes_written++;
  }
  wrote = self->intrf->write(self->ctx, sizeof(end_char), &end_char);
  if (wrote < 0) {
    err = SLIP_ERR_WRITE_FAIL;
  }
exit:
  *size = bytes_written;
  return err;
}

enum slip_err slip_msg_deinit(struct slip_msg *self) {
  enum slip_err err = SLIP_ERR_OK;
  if (self->intrf->deinit(self->ctx) < 0) {
    err = SLIP_ERR_DEINIT_FAIL;
  }
  return err;
}
