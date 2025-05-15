
#include "slip_msg.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
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
  int fd = open("dump.txt", O_RDONLY);
  assert(fd >= 0);
  struct slip_msg slip_msg = {.intrf = &file_intrf,
                              .ctx = (void *)(intptr_t)fd};

  struct slip_frame frame = {0};
  enum slip_err err = slip_msg_read(&slip_msg, &frame);

  if (err != SLIP_ERR_OK) {
    fprintf(stderr, "error: %d\n", err);
  }

  uint8_t str[frame.size + 1];
  memcpy(str, frame.data, frame.size);
  str[frame.size] = '\0';
  printf("data: %s\n", str);

  close(fd);

  return 0;
}
