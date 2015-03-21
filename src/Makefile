CC=gcc
CFLAGS=-c -Wall -O1 -DMPU9250 -DEMPL_TARGET_PI -I/usr/local/include
LFLAGS=-L/usr/local/lib -lnanomsg

all: motioncontrold

motioncontrold: pi.o inv_mpu.o inv_mpu_dmp.o motion.o main.o
	$(CC) $(LFLAGS) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -o $@

motion.o: motion.c
	$(CC) $(CFLAGS) $^ -o $@

inv_mpu.o: driver/eMPL/inv_mpu.c
	$(CC) $(CFLAGS) $^ -o $@

inv_mpu_dmp.o: driver/eMPL/inv_mpu_dmp_motion_driver.c
	$(CC) $(CFLAGS) $^ -o $@

pi.o: driver/eMPL/pi.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -f *.o core motioncontrold
