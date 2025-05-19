
#include "slip_msg.h"
#include <stdint.h>
#include <unistd.h>

int32_t write_file(void *ctx, uint16_t size, uint8_t const buffer[size]) {
  int fd = (intptr_t)ctx;
  return write(fd, buffer, size);
}

int32_t read_file(void *ctx, uint16_t size, uint8_t buffer[size]) {
  int fd = (intptr_t)ctx;
  return read(fd, buffer, size);
}

int8_t file_deinit(void *ctx) {
  int fd = (intptr_t)ctx;
  return (int8_t)close(fd);
}

struct slip_msg_intrf fd_intrf = {
    .write = write_file,
    .read = read_file,
    .deinit = file_deinit,
};
