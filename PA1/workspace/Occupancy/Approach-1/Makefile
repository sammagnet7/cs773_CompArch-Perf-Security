# Compiler
CC = gcc

# Source files
SRCS_A = occ_cal_200_mr_avg.c
SRCS_B = thrasher_cal.c
SRCS_SENDER = sender.c
SRCS_RECV = receiver.c
# Output Executables
OUT_A = occ_cal
OUT_B = thrash_cal
SENDER = sender
RECEIVER  = receiver

# Default target: Build both programs
all: $(OUT_A) $(OUT_B) $(SENDER) $(RECEIVER)

# Rule to compile programA
$(OUT_A): $(SRCS_A)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile programB
$(OUT_B): $(SRCS_B)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile sender
$(SENDER): $(SRCS_SENDER)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile receiver
$(RECEIVER): $(SRCS_RECV)
	$(CC) $(CFLAGS) -o $@ $^

# Clean compiled files
clean:
	rm -f $(OUT_A) $(OUT_B) $(SENDER) $(RECEIVER)
