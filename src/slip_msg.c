
#include "slip_msg.h"

#define END 0300
#define ESC_END 0334

#define ESC 0333
#define ESC_ESC 0335

/* ============================================= Public functions definition */

enum slip_err slip_msg_read(struct slip_msg const *self,
                            struct slip_frame *frame) {
  enum slip_err err = SLIP_ERR_OK;
  frame->size = 0;
  uint8_t start_char;
  int32_t read = self->intrf->read(self->ctx, &start_char, sizeof(uint8_t));
  if (read < 0) {
    err = SLIP_ERR_READ_FAIL;
    goto exit;
  }
  if (start_char != END) {
    err = SLIP_ERR_PKG_INVALID_START;
    goto exit;
  }
  frame->size = 1;
  uint16_t cursor = 0;
  while (frame->size < SLIP_PKG_MAX_SIZE) {
    int32_t read =
        self->intrf->read(self->ctx, frame->data + cursor, sizeof(uint8_t));
    if (read < 0) {
      err = SLIP_ERR_READ_FAIL;
      goto exit;
    }
    switch (frame->data[cursor]) {
    case END:
      if (cursor == 0) {
        continue;
      }
      frame->size--;
      goto exit;
      break;
    case ESC:
      read =
          self->intrf->read(self->ctx, frame->data + cursor, sizeof(uint8_t));
      if (read < 0) {
        err = SLIP_ERR_READ_FAIL;
        goto exit;
      }
      switch (frame->data[cursor]) {
      case ESC_END:
        frame->data[cursor] = END;
        break;
      case ESC_ESC:
        frame->data[cursor] = ESC;
        break;
      default:
        err = SLIP_ERR_PKG_INVALID_ESC;
        goto exit;
      }
      break;
    default:
      break;
    }
    cursor++;
    frame->size++;
  }
  if (frame->size > SLIP_PKG_MAX_SIZE) {
    err = SLIP_ERR_PKG_TOO_BIG;
  }
exit:
  return err;
}

enum slip_err slip_msg_write(struct slip_msg const *self, uint8_t const *buffer,
                             uint16_t size) {
  enum slip_err err = SLIP_ERR_OK;
  if (size > SLIP_PKG_MAX_SIZE) {
    // TODO: break up the payload in multiple slip frames.
    err = SLIP_ERR_PKG_TOO_BIG;
    goto exit;
  }
  static uint8_t const end_char = END;
  static uint8_t const esc_end_seq[] = {ESC, ESC_END};
  static uint8_t const esc_esc_seq[] = {ESC, ESC_ESC};
  int32_t wrote = self->intrf->write(self->ctx, &end_char, sizeof(end_char));
  if (wrote < 0) {
    err = SLIP_ERR_WRITE_FAIL;
    goto exit;
  }
  uint16_t cursor = 0;
  while (size > 0) {
    switch (buffer[cursor]) {
    case END:
      wrote = self->intrf->write(self->ctx, esc_end_seq, sizeof(esc_end_seq));
      break;
    case ESC:
      wrote = self->intrf->write(self->ctx, esc_esc_seq, sizeof(esc_esc_seq));
      break;
    default:
      wrote = self->intrf->write(self->ctx, buffer + cursor, sizeof(uint8_t));
    }
    if (wrote < 0) {
      err = SLIP_ERR_WRITE_FAIL;
      goto exit;
    }
    size--;
    cursor++;
  }
  wrote = self->intrf->write(self->ctx, &end_char, sizeof(end_char));
  if (wrote < 0) {
    err = SLIP_ERR_WRITE_FAIL;
  }
exit:
  return err;
}

enum slip_err slip_msg_deinit(struct slip_msg *self) {
  enum slip_err err = SLIP_ERR_OK;
  if (self->intrf->deinit(self->ctx) < 0) {
    err = SLIP_ERR_DEINIT_FAIL;
  }
  return err;
}
