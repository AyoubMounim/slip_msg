
LIB_NAME = slip
INSTALL_PREFIX = /usr/local

INCLUDE_DIR = include
SRC_DIR = src
BUILD_DIR = build
TEST_SRC_DIR = test
TEST_BUILD_DIR = $(BUILD_DIR)/test

SRCS = $(shell find $(SRC_DIR) -name "*.c")
TEST_SRCS = $(shell find $(TEST_SRC_DIR) -name "*.c")
TEST_COMMON_SRCS = $(TEST_SRC_DIR)/slip_fd_intrf.c

OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
TEST_OBJS = $(patsubst $(TEST_SRC_DIR)/%.c,$(TEST_BUILD_DIR)/%.o,$(TEST_SRCS))
TEST_COMMON_OBJS = $(patsubst $(TEST_SRC_DIR)/%.c,$(TEST_BUILD_DIR)/%.o,$(TEST_COMMON_SRCS))

CFLAGS = -I$(INCLUDE_DIR)
LDFLAGS = -L$(BUILD_DIR) -l$(LIB_NAME)
ARFLAGS = rc

TEST_EXE = $(TEST_BUILD_DIR)/write_on_file $(TEST_BUILD_DIR)/read_on_file $(TEST_BUILD_DIR)/echo_server $(TEST_BUILD_DIR)/echo_client

.PHONY = all test clean library install

all: library test

library: $(BUILD_DIR)/lib$(LIB_NAME).a

test: $(TEST_EXE)

$(BUILD_DIR)/lib$(LIB_NAME).a: $(OBJS) $(INCLUDE_DIR)/slip_msg.h
	$(AR) $(ARFLAGS) $@ $(OBJS)

$(TEST_BUILD_DIR)/write_on_file: $(TEST_BUILD_DIR)/write_on_file.o $(TEST_COMMON_OBJS) library
	$(CC) -o $@ $< $(LDFLAGS)

$(TEST_BUILD_DIR)/read_on_file: $(TEST_BUILD_DIR)/read_on_file.o $(TEST_COMMON_OBJS) library
	$(CC)  -o $@ $< $(LDFLAGS)

$(TEST_BUILD_DIR)/echo_server: $(TEST_BUILD_DIR)/echo_server.o $(TEST_COMMON_OBJS) library
	$(CC) -o $@ $< $(LDFLAGS)

$(TEST_BUILD_DIR)/echo_client: $(TEST_BUILD_DIR)/echo_client.o $(TEST_COMMON_OBJS) library
	$(CC) -o $@ $< $(LDFLAGS)

$(TEST_BUILD_DIR)/%.o: $(TEST_SRC_DIR)/%.c $(INCLUDE_DIR)/slip_msg.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDE_DIR)/slip_msg.h
	$(CC) $(CFLAGS) -c -o $@ $<

install: library
	[ -d $(INSTALL_PREFIX)/include ] || mkdir -p $(INSTALL_PREFIX)/include
	[ -d $(INSTALL_PREFIX)/lib ] || mkdir -p $(INSTALL_PREFIX)/lib
	cp $(INCLUDE_DIR)/* $(INSTALL_PREFIX)/include
	cp $(BUILD_DIR)/lib$(LIB_NAME).a $(INSTALL_PREFIX)/lib

clean:
	rm -rf $(BUILD_DIR)

$(info $(shell mkdir -p $(BUILD_DIR) $(TEST_BUILD_DIR)))

