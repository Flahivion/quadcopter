#include "motion.h"

int motion_init(const char* i2cdevice, motion_context* context)
{
	struct int_param_s int_param;
	
	context->accelValues[0] = context->accelValues[1] = context->accelValues[2] = 0.0f;
	context->gyroValues[0] = context->gyroValues[1] = context->gyroValues[2] = 0.0f;
	context->compassValues[0] = context->compassValues[1] = context->compassValues[2] = 0.0f;
	context->pitch = context->roll = context->yaw = 0.0f;
	context->accelOffset[0] = context->accelOffset[1] = context->accelOffset[2] = 0.0f;
	context->gyroOffset[0] = context->gyroOffset[1] = context->gyroOffset[2] = 0.0f;

	
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
	
	if (motion_calibrate(context->accelOffset, context->gyroOffset) != 0)
	{
		printf("Failed to calibrate sensors.\n");
		return -1;
	}
		
	return 0;
}


int motion_process(motion_context* context, float time_delta)
{
	int result;
	float gyroX, gyroY, gyroZ;
	
	result = motion_sample(context->accelValues, context->gyroValues, context->compassValues);
	
	if (result & SAMPLE_ACCEL)
	{
		context->accelValues[0] -= context->accelOffset[0];
		context->accelValues[1] -= context->accelOffset[1];
		context->accelValues[2] -= context->accelOffset[2];
	}
	
	if (result & SAMPLE_GYRO)
	{
		context->gyroValues[0] -= context->gyroOffset[0];
		context->gyroValues[1] -= context->gyroOffset[1];
		context->gyroValues[2] -= context->gyroOffset[2];
	}

	gyroX = context->gyroValues[0];
	gyroY = context->gyroValues[1];
	gyroZ = context->gyroValues[2];

	if (gyroX > GYRO_CUTOFF_X_LOW && gyroX < GYRO_CUTOFF_X_HIGH)
		gyroX = 0.0f;

	if (gyroY > GYRO_CUTOFF_Y_LOW && gyroY < GYRO_CUTOFF_Y_HIGH)
		gyroY = 0.0f;

	if (gyroZ > GYRO_CUTOFF_Z_LOW && gyroZ < GYRO_CUTOFF_Z_HIGH)
		gyroZ = 0.0f;

	/* IMU is mounted such that:
	     - X axis points to the front
	     - Y axis points to the right
	     - Z axis points down
	*/

	context->roll += gyroX * time_delta;
	context->pitch += gyroY * time_delta;
	context->yaw += gyroZ * time_delta;
	
	return result;
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

	accelOffset[0] = (accel[0] / (float)samples) / (float)ACCEL_SENSITIVITY;
	accelOffset[1] = (accel[1] / (float)samples) / (float)ACCEL_SENSITIVITY;
	accelOffset[2] = (accel[2] / (float)samples) / (float)ACCEL_SENSITIVITY;
	
	gyroOffset[0] = (gyro[0] / (float)samples) / (float)GYRO_SENSITIVITY;
	gyroOffset[1] = (gyro[1] / (float)samples) / (float)GYRO_SENSITIVITY;
	gyroOffset[2] = (gyro[2] / (float)samples) / (float)GYRO_SENSITIVITY;
	
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
	
	accel[0] = accel_cur[0] / (float)ACCEL_SENSITIVITY;
	accel[1] = accel_cur[1] / (float)ACCEL_SENSITIVITY;
	accel[2] = accel_cur[2] / (float)ACCEL_SENSITIVITY;
	
	return 0;
}

int motion_gyro_sample(float* gyro)
{
	short gyro_cur[3];
	
	if (mpu_get_gyro_reg(gyro_cur, NULL) != 0)
		return -1;
	
	gyro[0] = gyro_cur[0] / (float)GYRO_SENSITIVITY;
	gyro[1] = gyro_cur[1] / (float)GYRO_SENSITIVITY;
	gyro[2] = gyro_cur[2] / (float)GYRO_SENSITIVITY;
	
	return 0;
}

int motion_compass_sample(float* compass)
{
	short compass_cur[3];
	
	if (mpu_get_compass_reg(compass_cur, NULL) != 0)
		return -1;
	
	compass[0] = compass_cur[0] / (float)COMPASS_SENSITIVITY;
	compass[1] = compass_cur[1] / (float)COMPASS_SENSITIVITY;
	compass[2] = compass_cur[2] / (float)COMPASS_SENSITIVITY;
	
	return 0;
}

void motion_close()
{
	if (i2c_close() != 0)
	{
		printf("Failed to close I2C bus.\n");
	}
}
