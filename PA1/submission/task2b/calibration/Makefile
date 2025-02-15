# Compiler
CC = gcc
CFLAGS = -I ../include/
# Source files
SRCS_occ = occ_cal_200_mr_median_300.c
SRCS_thr = thrasher_cal.c

# Output Executables
TARGETS = occ_cal thrash_cal

# Default target
all: $(TARGETS)

# Rule to compile programA
occ_cal: $(SRCS_occ)
	$(CC) $(CFLAGS) -o $@ $^ 

# Rule to compile programB
thrash_cal: $(SRCS_thr)
	$(CC) $(CFLAGS) -o $@ $^


clean:
	rm -f $(TARGETS)
