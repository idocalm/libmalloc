CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
INCLUDES = -I. -I./internals

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CFLAGS += -D_DEBUG=1
endif

SRCS = $(wildcard *.c) $(wildcard */*.c) $(wildcard */*/*.c)
OBJS = $(SRCS:.c=.o)

LIBRARY = libmalloc.a

all: $(LIBRARY)

$(LIBRARY): $(OBJS)
	ar rcs $@ $^
	@echo "Built $(LIBRARY)"

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(LIBRARY)
	@echo "Cleaned"

rebuild: clean all
.PHONY: all clean rebuild

