CC = gcc
CFLAGS = -Wall

# Targets
all: sender receiver libshared.so

# Build sender executable
sender: sender.o utils.o
	$(CC) $(CFLAGS) -o sender sender.o utils.o

# Build receiver executable
receiver: receiver.o utils.o
	$(CC) $(CFLAGS) -o receiver receiver.o utils.o

# Create shared library
libshared.so: shared.o
	$(CC) -g -fPIC -shared -o libshared.so shared.o

# Compile utils.c to utils.o
utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c -o utils.o

# Compile sender.c to sender.o
sender.o: sender.c utils.h
	$(CC) $(CFLAGS) -c sender.c -o sender.o

# Compile receiver.c to receiver.o
receiver.o: receiver.c utils.h
	$(CC) $(CFLAGS) -c receiver.c -o receiver.o

# Compile shared.c to shared.o
shared.o: shared.c
	$(CC) $(CFLAGS) -fPIC -c shared.c -o shared.o

# Clean up build artifacts
clean:
	rm -f sender receiver libshared.so *.o