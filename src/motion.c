#include "motion.h"

int motion_init(const char* i2cdevice)
{
	struct int_param_s int_param;
	
	if (i2c_open(i2cdevice) != 0)
	{
		printf("Failed to open I2C bus.\n");
		return -1;
	}
	
	if (mpu_init(&int_param) != 0)
	{
		printf("Failed to initialize MPU.\n");
		return -1;
	}
	
	if (mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS) != 0)
	{
		printf("Failed to enable MPU sensors.\n");
		return -1;
	}
	
	if (mpu_set_gyro_fsr(GYRO_FSR) != 0)
	{
		printf("Failed to set gyro FSR.\n");
		return -1;
	}
	
	if (mpu_set_accel_fsr(ACCEL_FSR) != 0)
	{
		printf("Failed to set accel FSR.\n");
		return -1;
	}
	
	if (mpu_set_sample_rate(100) != 0)
	{
		printf("Failed to set sampling rate.\n");
		return -1;
	}
	
	if (mpu_set_compass_sample_rate(100) != 0)
	{
		printf("Failed to set compass sampling rate.\n");
		return -1;
	}
	
	/*if (motion_calibrate(_accelOffset, _gyroOffset) != 0)
	{
		printf("Failed to calibreate MPU.\n");
		return -1;
	}
	
	printf("Accelerometer offset: %7.4f %7.4f %7.4f\n", _accelOffset[0], _accelOffset[1], _accelOffset[2]);
	printf("Gyroscope offset: %7.4f %7.4f %7.4f\n", _gyroOffset[0], _gyroOffset[1], _gyroOffset[2]);*/
	
	return 0;
}

int motion_calibrate(float* accelOffset, float* gyroOffset)
{
	int samples = 0;
	long accel[3], gyro[3];
	short accel_cur[3], gyro_cur[3];
	
	accel[0] = accel[1] = accel[2] = 0;
	gyro[0] = gyro[1] = gyro[2] = 0;

	while (samples < NUM_CALIBRATE_SAMPLES)
	{
		if (mpu_get_accel_reg(accel_cur, NULL) != 0)
			return -1;
		
		if (mpu_get_gyro_reg(gyro_cur, NULL) != 0)
			return -1;

		accel[0] += (long)accel_cur[0];
		accel[1] += (long)accel_cur[1];
		accel[2] += (long)accel_cur[2];
			
		gyro[0] += (long)gyro_cur[0];
		gyro[1] += (long)gyro_cur[1];
		gyro[2] += (long)gyro_cur[2];
		
		samples++;
	}

	accelOffset[0] = (accel[0] / (float)samples) / (float)ACCEL_FSR;
	accelOffset[1] = (accel[1] / (float)samples) / (float)ACCEL_FSR;
	accelOffset[2] = (accel[2] / (float)samples) / (float)ACCEL_FSR;
	
	gyroOffset[0] = (gyro[0] / (float)samples) / (float)GYRO_FSR;
	gyroOffset[1] = (gyro[1] / (float)samples) / (float)GYRO_FSR;
	gyroOffset[2] = (gyro[2] / (float)samples) / (float)GYRO_FSR;
	
	return 0;
}

int motion_sample(float* accel, float* gyro, float* compass)
{
	int result = 0;
	
	if (motion_accel_sample(accel) != 0)
		printf("Failed to sample accelerometer data.\n");
	else
		result |= SAMPLE_ACCEL;
	
	if (motion_gyro_sample(gyro) != 0)
		printf("Failed to sample gyroscope data.\n");
	else
		result |= SAMPLE_GYRO;
		
	if (motion_compass_sample(compass) != 0)
		printf("Failed to sample compass data.\n");
	else
		result |= SAMPLE_COMPASS;
	
	return result;
}

int motion_accel_sample(float* accel)
{
	short accel_cur[3];
	
	if (mpu_get_accel_reg(accel_cur, NULL) != 0)
		return -1;
	
	accel[0] = accel_cur[0] / (float)ACCEL_FSR;
	accel[1] = accel_cur[1] / (float)ACCEL_FSR;
	accel[2] = accel_cur[2] / (float)ACCEL_FSR;
	
	return 0;
}

int motion_gyro_sample(float* gyro)
{
	short gyro_cur[3];
	
	if (mpu_get_gyro_reg(gyro_cur, NULL) != 0)
		return -1;
	
	gyro[0] = gyro_cur[0] / (float)GYRO_FSR;
	gyro[1] = gyro_cur[1] / (float)GYRO_FSR;
	gyro[2] = gyro_cur[2] / (float)GYRO_FSR;
	
	return 0;
}

int motion_compass_sample(float* compass)
{
	short compass_cur[3];
	
	if (mpu_get_compass_reg(compass_cur, NULL) != 0)
		return -1;
	
	compass[0] = compass_cur[0] / (float)COMPASS_FSR;
	compass[1] = compass_cur[1] / (float)COMPASS_FSR;
	compass[2] = compass_cur[2] / (float)COMPASS_FSR;
	
	return 0;
}

void motion_close()
{
	if (i2c_close() != 0)
	{
		printf("Failed to close I2C bus.\n");
	}
}
