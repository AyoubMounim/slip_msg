
#include "slip_msg.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int32_t write_file(void *ctx, uint8_t const *buffer, uint16_t size) {
  int fd = (intptr_t)ctx;
  return write(fd, buffer, size);
}

int32_t read_file(void *ctx, uint8_t *buffer, uint16_t size) {
  int fd = (intptr_t)ctx;
  return read(fd, buffer, size);
}

struct slip_msg_intrf file_intrf = {
    .write = write_file,
    .read = read_file,
};

int main(int argc, char *argv[]) {
  int fd = open("dump.txt", O_WRONLY);
  assert(fd >= 0);
  struct slip_msg slip_msg = {.intrf = &file_intrf,
                              .ctx = (void *)(intptr_t)fd};

  uint8_t data[] = "Hello\xc0 There!\n";
  enum slip_err err = slip_msg_write(&slip_msg, data, sizeof(data));

  if (err != SLIP_ERR_OK) {
    fprintf(stderr, "error: %d\n", err);
  }

  close(fd);

  return 0;
}
