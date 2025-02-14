# Compiler
CC = gcc
CFLAGS = -I ../include/
# Source files
SRCS_SENDER = sender_dynamic.c
SRCS_RECV = receiver_median.c

# Output Executables
TARGETS = sender receiver

# Default target: Build both programs
all: $(TARGETS)

# Rule to compile sender
sender: $(SRCS_SENDER)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile receiver
receiver: $(SRCS_RECV)
	$(CC) $(CFLAGS) -o $@ $^

# Clean compiled files
clean:
	rm -f $(TARGETS)
