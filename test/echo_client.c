
#include "slip_fd_intrf.c"
#include "slip_msg.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

struct prog_args {
  char const *sock_path;
};

static int prog_args_parse(struct prog_args *args, int argc, char *argv[]) {
  if (argc != 2) {
    return -1;
  }
  if (strcmp("--help", argv[1]) == 0) {
    return 1;
  }
  args->sock_path = argv[1];
  return 0;
}

static void print_usage(char const *prog_name) {
  printf("Usage: %s <unix_sock_path>\n", prog_name);
  return;
}

int main(int argc, char *argv[]) {
  struct prog_args args = {0};
  int parse_err = prog_args_parse(&args, argc, argv);
  if (parse_err < 0 || parse_err == 1) {
    print_usage(argv[0]);
    exit(1);
  }
  int sock = socket(AF_UNIX, SOCK_STREAM, 0);
  assert(sock >= 0);
  struct sockaddr_un sock_addr = {0};
  sock_addr.sun_family = AF_UNIX;
  strncpy(sock_addr.sun_path, args.sock_path, sizeof(sock_addr.sun_path) - 1);
  if (connect(sock, (const struct sockaddr *)&sock_addr, sizeof(sock_addr)) ==
      -1) {
    perror("connect");
    goto exit;
  }

  struct slip_msg msg = {
      .intrf = &fd_intrf,
      .ctx = (void *)(intptr_t)sock,
  };

  char *line = NULL;
  size_t line_capacity;
  ssize_t line_size;
  uint8_t read_buffer[SLIP_PKG_MAX_SIZE] = {0};
  while (1) {
    printf("\n> ");
    if ((line_size = getline(&line, &line_capacity, stdin)) < 0) {
      printf("getline error\n");
      free(line);
      line = NULL;
      continue;
    }
    if (line_size == 0) {
      continue;
    }
    uint16_t data_size = line_size - 1;
    enum slip_err err = slip_msg_write(&msg, &data_size, (uint8_t *)line);
    if (err != SLIP_ERR_OK) {
      printf("send error\n");
      continue;
    }
    uint16_t read_size = SLIP_PKG_MAX_SIZE;
    err = slip_msg_read(&msg, &read_size, read_buffer);
    if (err != SLIP_ERR_OK) {
      printf("read error\n");
      continue;
    }
    printf("\"%.*s\"\n", read_size, read_buffer);
  }

exit:
  free(line);
  close(sock);

  return 0;
}
