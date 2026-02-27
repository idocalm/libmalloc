CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -g -O0 -D_DEBUG=1
INCLUDES = -I../src -I../src/internals

INTERNAL_SRCS = \
	../src/internals/chunk.c \
	../src/internals/debug.c \
	../src/internals/ops.c \
	../src/internals/state.c \
	../src/internals/utils.c

TEST_SRCS = \
	test_main.c \
	test_bounds.c \
	test_alloc_free.c \
	test_realloc.c \
	test_zero_on_free.c \
	test_magic_cookie.c

TARGET = test_allocator

all: $(TARGET)

$(TARGET): $(TEST_SRCS) tests.h $(INTERNAL_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $(TEST_SRCS) $(INTERNAL_SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
