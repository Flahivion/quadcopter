#ifndef _MOTION_H_
#define _MOTION_H_

#include "driver/eMPL/inv_mpu.h"
#include "driver/eMPL/pi.h"

#define NUM_CALIBRATE_SAMPLES 200
#define GYRO_FSR 250 // 250dps
#define ACCEL_FSR 2 // 2G
#define COMPASS_FSR 4915 // 4915?'s

#define SAMPLE_ACCEL 0x01
#define SAMPLE_GYRO 0x02
#define SAMPLE_COMPASS 0x04

int motion_init(const char* i2cdevice);
int motion_calibrate();
int motion_sample(float* accel, float* gyro, float* compass);
int motion_accel_sample(float* accel);
int motion_gyro_sample(float* gyro);
int motion_compass_sample(float* compass);
void motion_close();

#endif
