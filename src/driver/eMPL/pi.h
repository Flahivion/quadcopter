#ifndef _PI_H
#define _PI_H

#include <stdio.h>
#include <unistd.h>
#include "inv_mpu.h"

int i2c_open(const char* filename);
int i2c_close();

int i2c_read(unsigned char slave_addr, unsigned char reg_addr,
	unsigned char length, unsigned char *data);

int i2c_write(unsigned char slave_addr, unsigned char reg_addr,
	unsigned char length, unsigned char const *data);

void delay_ms(unsigned long num_ms);
int get_ms(unsigned long* count);

int reg_int_cb(struct int_param_s *int_param);

void __no_operation();

#define log_i     printf
#define log_e     printf
#define min(a,b)       ((a<b)?a:b)

#endif
