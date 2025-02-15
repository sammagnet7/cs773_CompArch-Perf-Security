# Compiler
CC = gcc
CFLAGS = -Wall -O0 -g -Iinclude

# Source files
COMMON_SRC = utils.c
SENDER_SRC = sender.c $(COMMON_SRC)
RECEIVER_SRC = receiver.c $(COMMON_SRC)

# Object files
OBJ_DIR = obj
SENDER_OBJ = $(OBJ_DIR)/sender.o $(OBJ_DIR)/utils.o
RECEIVER_OBJ = $(OBJ_DIR)/receiver.o $(OBJ_DIR)/utils.o

# Executables
BIN_DIR = bin
SENDER_EXE = $(BIN_DIR)/sender
RECEIVER_EXE = $(BIN_DIR)/receiver

all: $(SENDER_EXE) $(RECEIVER_EXE)

# Create directories if they don't exist
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Rules

$(SENDER_EXE): $(OBJ_DIR) $(BIN_DIR) $(SENDER_OBJ)
	$(CC) $(CFLAGS) -o $@ $(SENDER_OBJ)

$(RECEIVER_EXE): $(OBJ_DIR) $(BIN_DIR) $(RECEIVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $(RECEIVER_OBJ)

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
