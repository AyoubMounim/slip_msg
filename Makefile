
LIB_NAME = slip

INCLUDE_DIR = include
SRC_DIR = src
BUILD_DIR = build
TEST_SRC_DIR = test
TEST_BUILD_DIR = $(BUILD_DIR)/test

SRCS = $(shell find $(SRC_DIR) -name "*.c")
TEST_SRCS = $(shell find $(TEST_SRC_DIR) -name "*.c")

OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
TEST_OBJS = $(patsubst $(TEST_SRC_DIR)/%.c,$(TEST_BUILD_DIR)%.o,$(TEST_SRCS))

CFLAGS = -I$(INCLUDE_DIR)
LDFLAGS = -L$(BUILD_DIR) -l$(LIB_NAME)
ARFLAGS = rc

TEST_EXE = $(TEST_BUILD_DIR)/write_on_file $(TEST_BUILD_DIR)/read_on_file $(TEST_BUILD_DIR)/echo_server $(TEST_BUILD_DIR)/echo_client

.PHONY = all test clean library

all: library test

library: $(BUILD_DIR)/lib$(LIB_NAME).a

test: $(TEST_EXE)

$(BUILD_DIR)/lib$(LIB_NAME).a: $(OBJS) $(INCLUDE_DIR)/slip_msg.h
	$(AR) $(ARFLAGS) $@ $(OBJS)

$(TEST_BUILD_DIR)/write_on_file: $(TEST_BUILD_DIR)/write_on_file.o library
	$(CC) -o $@ $< $(LDFLAGS)

$(TEST_BUILD_DIR)/read_on_file: $(TEST_BUILD_DIR)/read_on_file.o library
	$(CC)  -o $@ $< $(LDFLAGS)

$(TEST_BUILD_DIR)/echo_server: $(TEST_BUILD_DIR)/echo_server.o library
	$(CC) -o $@ $< $(LDFLAGS)

$(TEST_BUILD_DIR)/echo_client: $(TEST_BUILD_DIR)/echo_client.o library
	$(CC) -o $@ $< $(LDFLAGS)

$(TEST_BUILD_DIR)/%.o: $(TEST_SRC_DIR)/%.c $(INCLUDE_DIR)/slip_msg.h
	gcc $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDE_DIR)/slip_msg.h
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

$(info $(shell mkdir -p $(BUILD_DIR) $(TEST_BUILD_DIR)))

