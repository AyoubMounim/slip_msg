
#include "slip_msg.h"
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static int sock;
static int data_sock;
static char const *sock_path;

static void sigint_handler(int signum) {
  if (signum != SIGINT) {
    return;
  }
  unlink(sock_path);
  close(sock);
  close(data_sock);
  exit(0);
}

static struct sigaction sigact = {.sa_handler = sigint_handler};

static int32_t write_file(void *ctx, uint8_t const *buffer, uint16_t size) {
  int fd = (intptr_t)ctx;
  return write(fd, buffer, size);
}

static int32_t read_file(void *ctx, uint8_t *buffer, uint16_t size) {
  int fd = (intptr_t)ctx;
  return read(fd, buffer, size);
}

static struct slip_msg_intrf file_intrf = {
    .write = write_file,
    .read = read_file,
};

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
  sock_path = args.sock_path;
  assert(sigaction(SIGINT, &sigact, NULL) == 0);
  sock = socket(AF_UNIX, SOCK_STREAM, 0);
  assert(sock >= 0);
  struct sockaddr_un sock_addr = {0};
  sock_addr.sun_family = AF_UNIX;
  strncpy(sock_addr.sun_path, sock_path, sizeof(sock_addr.sun_path) - 1);
  if (bind(sock, (const struct sockaddr *)&sock_addr, sizeof(sock_addr)) ==
      -1) {
    perror("bind");
    close(sock);
    return 1;
  }
  if (listen(sock, 0) < 0) {
    perror("listen");
    close(sock);
    unlink(sock_path);
    return 1;
  }
  struct sockaddr_un client_addr = {0};
  socklen_t client_addr_len;
accept:
  while (1) {
    printf("\nwaiting for connections...\n");
    if ((data_sock = accept(sock, (struct sockaddr *)&client_addr,
                            &client_addr_len)) < 0) {
      perror("accept");
      continue;
    }
    printf("connected\n");
    break;
  }

  struct slip_msg msg = {
      .intrf = &file_intrf,
      .ctx = (void *)(intptr_t)data_sock,
  };
  struct slip_frame frame = {0};
  while (1) {
    printf("\nwaiting for data...\n");
    enum slip_err err = slip_msg_read(&msg, &frame);
    if (err != SLIP_ERR_OK) {
      printf("recv error\n");
      close(data_sock);
      goto accept;
    }
    if (frame.size >= SLIP_PKG_MAX_SIZE) {
      printf("too big\n");
      continue;
    }
    frame.data[frame.size] = '\0';
    printf("recv: \"%s\"\n", frame.data);
    err = slip_msg_write(&msg, frame.data, frame.size);
    if (err != SLIP_ERR_OK) {
      printf("send error\n");
    }
  }

  return 0;
}
