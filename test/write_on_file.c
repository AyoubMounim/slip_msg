
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
  char const *data_string;
};

static int prog_args_parse(struct prog_args *args, int argc, char *argv[]) {
  if (argc == 2) {
    if (strcmp("--help", argv[1]) == 0) {
      return 1;
    }
    return -1;
  }
  if (argc != 3) {
    return -1;
  }
  args->file_path = argv[1];
  args->data_string = argv[2];
  return 0;
}

static void print_usage(char const *prog_name) {
  printf("Usage: %s <file_path> <data_string>\n", prog_name);
  return;
}

int main(int argc, char *argv[]) {
  struct prog_args args = {0};
  int parse_err = prog_args_parse(&args, argc, argv);
  if (parse_err < 0 || parse_err == 1) {
    print_usage(argv[0]);
    exit(1);
  }
  int fd = open(args.file_path, O_WRONLY | O_CREAT | O_TRUNC);
  assert(fd >= 0);
  struct slip_msg slip_msg = {
      .intrf = &fd_intrf,
      .ctx = (void *)(intptr_t)fd,
  };

  uint16_t str_len = strlen(args.data_string);
  uint16_t data_size = str_len;
  enum slip_err err =
      slip_msg_write(&slip_msg, &data_size, (uint8_t *)args.data_string);

  if (err != SLIP_ERR_OK) {
    fprintf(stderr, "error: %d\n", err);
  } else if (data_size != str_len) {
    fprintf(stderr, "wrong written size, expected: %u, got: %u\n", str_len,
            data_size);
  }

  slip_msg_deinit(&slip_msg);

  return 0;
}
