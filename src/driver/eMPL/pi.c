#include "pi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>

static int _i2c_file = -1;

int i2c_open(const char* filename)
{
	int file;
	
	file = open(filename, O_RDWR);
	if (file < 0)
	{
		printf("Error opening i2c bus.\n");
		return -1;
	}
	
	_i2c_file = file;
	return 0;
}

int i2c_close()
{
	if (_i2c_file >= 0)
	{
		if (close(_i2c_file) < 0)
			return -1;
	}

	return 0;
}

int i2c_read(unsigned char slave_addr, unsigned char reg_addr,
	unsigned char length, unsigned char *data)
{
	struct i2c_rdwr_ioctl_data combined;
	struct i2c_msg msgs[2];

	if (length < 1)
		return -1;
	
	combined.msgs = msgs;
	combined.nmsgs = 2;
	
	// Address of register to read.
	msgs[0].addr = slave_addr;
	msgs[0].flags = 0;
	msgs[0].buf = (char*)&reg_addr;
	msgs[0].len = 1;
	
	// Data to read.
	msgs[1].addr = slave_addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].buf = (char*)data;
	msgs[1].len = length;
	
	if (ioctl(_i2c_file, I2C_RDWR, &combined) < 0)
		return -1;
	
	return 0;
}

int i2c_write(unsigned char slave_addr, unsigned char reg_addr,
	unsigned char length, unsigned char const *data)
{
	char* buffer;
	
	if (length < 1)
		return -1;
	
	if (ioctl(_i2c_file, I2C_SLAVE, slave_addr) < 0)
		return -1;
	
	buffer = (char*)malloc(length + 1);
	memcpy(buffer + 1, data, length);
	buffer[0] = reg_addr;
	
	if (write(_i2c_file, buffer, length + 1) != length + 1)
	{
		free(buffer);
		return -1;
	}

	free(buffer);
	
	return 0;
}

void delay_ms(unsigned long num_ms)
{
	usleep(num_ms * 1000);
}

int get_ms(unsigned long* count)
{
	/*struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
	{
		log_e("Error retrieving time in ms");
		return 0;
	}
	
	return ts.tv_nsec / (1000 * 1000);*/
	return -1;
}

int reg_int_cb(struct int_param_s *int_param)
{
    return -1;
}

void __no_operation()
{
	do {} while(0);
}
