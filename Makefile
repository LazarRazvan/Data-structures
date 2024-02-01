# Compiler and compiler flags
CC = gcc
CFLAGS = -Wall -g
INCLUDES = -I./include

# Source and object files in src directory
SRCS := $(wildcard src/*/*.c)
OBJS := $(SRCS:.c=.o)

# Test source files and executables
TEST_SRCS := $(wildcard test/*.c)
TEST_EXE := $(TEST_SRCS:.c=)

# Default target
all: $(OBJS) $(TEST_EXE)

# Rule to make object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Rule to make test executables
test/%: test/%.c $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

# Clean target
clean:
	rm -f $(OBJS) $(TEST_EXE)

.PHONY: all clean

