CC=gcc
CFLAGS=-c -Wall -O1 -DMPU9250 -DEMPL_TARGET_PI -I../lib
LFLAGS=-lssl -lz -lcrypto -lrt

all: motioncontrold

motioncontrold: bin/pi.o bin/inv_mpu.o bin/inv_mpu_dmp.o bin/motion.o bin/server.o bin/main.o ../lib/libwebsockets.a
	$(CC) $(LFLAGS) $^ -o bin/$@

bin/main.o: main.c
	$(CC) $(CFLAGS) $^ -o $@

bin/server.o: server.c
	$(CC) $(CFLAGS) $^ -o $@

bin/motion.o: motion.c
	$(CC) $(CFLAGS) $^ -o $@

bin/inv_mpu.o: driver/eMPL/inv_mpu.c
	$(CC) $(CFLAGS) $^ -o $@

bin/inv_mpu_dmp.o: driver/eMPL/inv_mpu_dmp_motion_driver.c
	$(CC) $(CFLAGS) $^ -o $@

bin/pi.o: driver/eMPL/pi.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -f bin/*.o bin/core bin/motioncontrold
