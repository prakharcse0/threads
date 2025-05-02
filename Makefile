CC = gcc
CFLAGS = -Wall -Wextra -g -pthread
LDFLAGS = -pthread

# Source files
SRC = my_semaphore.c my_recursive_mutex.c my_condition_variable.c thread_pool.c my_barrier.c example_program.c
OBJ = $(SRC:.c=.o)

# Header files
HEADERS = my_semaphore.h my_recursive_mutex.h my_condition_variable.h thread_pool.h my_barrier.h

# Executables
MAIN = thread_sync_demo

# Default target
all: $(MAIN)

# Linking
$(MAIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# Compilation
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Phony targets
.PHONY: clean test

# Clean up build files
clean:
	rm -f $(OBJ) $(MAIN)

# Run test program
test: $(MAIN)
	./$(MAIN)

# Install (optional, modify path as needed)
install: $(MAIN)
	cp $(MAIN) /usr/local/bin/
