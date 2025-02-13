CC = gcc
CFLAGS = -Wall -g

# Targets
all: sender receiver libshared.so calibrate

# Build sender executable
sender: sender.o utils/utils.o utils/cacheutils.o transmit_layer/transmit_layer.o common.o
	$(CC) $(CFLAGS) -o sender sender.o utils/utils.o utils/cacheutils.o transmit_layer/transmit_layer.o common.o

# Build receiver executable
receiver: receiver.o utils/utils.o utils/cacheutils.o transmit_layer/transmit_layer.o common.o
	$(CC) $(CFLAGS) -o receiver receiver.o utils/utils.o utils/cacheutils.o transmit_layer/transmit_layer.o common.o

calibrate: calibrate.o utils/utils.o utils/cacheutils.o transmit_layer/transmit_layer.o common.o
	$(CC) $(CFLAGS) -o calibrate calibrate.o utils/utils.o utils/cacheutils.o transmit_layer/transmit_layer.o common.o

# Create shared library
libshared.so: shared.o
	$(CC) -g -fPIC -shared -o libshared.so shared.o

# Compile utils.c to utils.o
utils/utils.o: utils/utils.c utils/utils.h
	$(CC) $(CFLAGS) -c utils/utils.c -o utils/utils.o

# Compile sender.c to sender.o
sender.o: sender.c
	$(CC) $(CFLAGS) -c sender.c -o sender.o

# Compile receiver.c to receiver.o
receiver.o: receiver.c
	$(CC) $(CFLAGS) -c receiver.c -o receiver.o

calibrate.o: calibrate.c
	$(CC) $(CFLAGS) -c calibrate.c -o calibrate.o

# Compile shared.c to shared.o
shared.o: shared.c
	$(CC) $(CFLAGS) -fPIC -c shared.c -o shared.o

# Compile transmit_layer.c to transmit_layer.o
transmit_layer/transmit_layer.o: transmit_layer/transmit_layer.c transmit_layer/transmit_layer.h
	$(CC) $(CFLAGS) -c transmit_layer/transmit_layer.c -o transmit_layer/transmit_layer.o

# Compile cacheutils.c to cacheutils.o
utils/cacheutils.o: utils/cacheutils.c utils/cacheutils.h
	$(CC) $(CFLAGS) -c utils/cacheutils.c -o utils/cacheutils.o

# Compile common.c to common.o
common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c -o common.o

# Clean up build artifacts
clean:
	rm -f sender receiver libshared.so *.o transmit_layer/*.o utils/*.o