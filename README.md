
# SLIP messenger

simple implementation of the SLIP transmission protocol (RFC 1055).

The implementation is agnostic on how data is actually written for
transmission, it relies on externally provided function through the `struct
slip_msg_intrf` structure (see test/slip_fd_intrf.c).

## Build

Run `make` in the root directory to build the library and the tests executables.
Run `make INSTALL_PREFIX=<prefix_path> install` to install the library into
`prefix_path` (defaults to /usr/local/).
