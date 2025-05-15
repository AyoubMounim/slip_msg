
#include "slip_fd_intrf.c"
#include "slip_msg.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct prog_args {
  char const *file_path;
};

static int prog_args_parse(struct prog_args *args, int argc, char *argv[]) {
  if (argc != 2) {
    return -1;
  }
  if (strcmp("--help", argv[1]) == 0) {
    return 1;
  }
  args->file_path = argv[1];
  return 0;
}

static void print_usage(char const *prog_name) {
  printf("Usage: %s <file_path>\n", prog_name);
  return;
}

int main(int argc, char *argv[]) {
  struct prog_args args = {0};
  int parse_err = prog_args_parse(&args, argc, argv);
  if (parse_err < 0 || parse_err == 1) {
    print_usage(argv[0]);
    exit(1);
  }
  int fd = open(args.file_path, O_RDONLY);
  assert(fd >= 0);
  struct slip_msg slip_msg = {
      .intrf = &fd_intrf,
      .ctx = (void *)(intptr_t)fd,
  };

  struct slip_frame frame = {0};
  enum slip_err err = slip_msg_read(&slip_msg, &frame);

  if (err != SLIP_ERR_OK) {
    fprintf(stderr, "error: %d\n", err);
  }

  uint8_t str[frame.size + 1];
  memcpy(str, frame.data, frame.size);
  str[frame.size] = '\0';
  printf("data: %s\n", str);

  slip_msg_deinit(&slip_msg);

  return 0;
}
