CC = gcc
CFLAGS = -Wall -g -Iinclude

TARGETS = sender receiver calibrate transcode

all: sender receiver

sender: source/sender.c utils.o
	$(CC) $(CFLAGS) -o sender source/sender.c utils.o

receiver: source/receiver.c utils.o
	$(CC) $(CFLAGS) -o receiver source/receiver.c utils.o

calibrate: source/calibrate.c utils.o
	$(CC) $(CFLAGS) -o calibrate source/calibrate.c utils.o

transcode: source/transcode.c
	$(CC) $(CFLAGS) -o transcode source/transcode.c

utils.o: source/utils.c include/utils.h
	$(CC) $(CFLAGS) -c source/utils.c -o utils.o

clean:
	rm -f $(TARGETS) utils.o
