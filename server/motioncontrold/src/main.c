#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include "serial.h"
#include "motion.h"
#include "server.h"

#define imin(x, y) (((x) < (y)) ? (x) : (y))
#define imax(x, y) (((x) > (y)) ? (x) : (y))

#define SEND_BUFFER_LENGTH 1024
#define CMD_SETPWMCHANNELS 0xD1

#define PID_PITCH_KP 0.5f
#define PID_PITCH_MAX 30.0f
#define PID_ROLL_KP 0.5f 
#define PID_ROLL_MAX 30.0f
#define SPEED_MAX 60.0f
#define SPEED_MIN 20.0f
#define SPEED_BASE 50.0f

void handle_sigint(int signal);
void process_data(
	server_context* server,
	motion_context* motion,
	int serial_fd,
	float time_total,
	float time_delta,
	char* buffer,
	int length);
void process_control(int serial_fd, motion_context* motion, unsigned int* rotors, float time_total, float time_delta);

static int _force_exit = 0;

int main(int argc, char* argv[])
{
	server_context server;
	char* send_buffer;
	struct timespec prev_time, cur_time;
	motion_context motion;
	float time_total;
	float time_delta;
	int serial_fd;

	if (argc != 4)
	{
		printf("Usage: motioncontrold <i2c_address> <websocket_port> <serial_device>\n");
		return EXIT_FAILURE;
	}
	
	signal(SIGINT, handle_sigint);
	
	if (motion_init(argv[1], &motion) != 0)
	{
		printf("Failed to initialize motion processing unit.\n");

		motion_close();
		return EXIT_FAILURE;
	}

	if (server_open(atoi(argv[2]), &server) != 0)
	{
		printf("Failed to initialize server on port %i\n", atoi(argv[2]));
		motion_close();
		return EXIT_FAILURE;
	}

	serial_fd = serial_open(argv[3]);
	if (serial_fd < 0)
	{
		printf("Failed to initialize serial console.\n");
		motion_close();
		return EXIT_FAILURE;
	}

	sleep(1);
	
	send_buffer = (char*)malloc(SEND_BUFFER_LENGTH);
	clock_gettime(CLOCK_MONOTONIC, &prev_time);
	time_total = 0.0f;
	time_delta = 0.0f;

	while (!_force_exit)
	{
		clock_gettime(CLOCK_MONOTONIC, &cur_time);
		time_delta = (cur_time.tv_sec - prev_time.tv_sec) + ((cur_time.tv_nsec - prev_time.tv_nsec) / 1000000000.0f);
		time_total += time_delta;
		prev_time = cur_time;
		
		process_data(&server, &motion, serial_fd, time_total, time_delta, send_buffer, SEND_BUFFER_LENGTH);
		server_process(&server);
	}

	server_close(&server);
	motion_close();
	serial_close(serial_fd);
	
	free(send_buffer);
	send_buffer = NULL;
	
	return EXIT_SUCCESS;
}

void handle_sigint(int signal)
{
	printf("SIGINT received, exiting...\n");
	_force_exit = 1;
}

void process_data(server_context* server, motion_context* motion, int serial_fd, float time_total, float time_delta, char* buffer, int length)
{
	char temp[255];
	unsigned int rotors[4];
	
	/* Process motion data */
	motion_process(motion, time_delta);

	/* Update control inputs */
	process_control(serial_fd, motion, rotors, time_total, time_delta);
	
	/* Construct message with data to send. */
	buffer[0] = '\0';
	
	sprintf(temp, "TS:%7.4f;", time_total);
	strcat(buffer, temp);
	
	sprintf(temp, "AX:%7.4f,AY:%7.4f,AZ:%7.4f;", motion->accelValues[0], motion->accelValues[1], motion->accelValues[2]);
	strcat(buffer, temp);

	sprintf(temp, "GX:%7.4f,GY:%7.4f,GZ:%7.4f;", motion->gyroValues[0], motion->gyroValues[1], motion->gyroValues[2]);
	strcat(buffer, temp);

	sprintf(temp, "CX:%7.4f,CY:%7.4f,CZ:%7.4f;", motion->compassValues[0], motion->compassValues[1], motion->compassValues[2]);
	strcat(buffer, temp);
	
	sprintf(temp, "OP:%7.4f,OR:%7.4f,OY:%7.4f;", motion->pitch, motion->roll, motion->yaw);
	strcat(buffer, temp);

	sprintf(temp, "R1:%d,R2:%d,R3:%d,R4:%d;", rotors[0], rotors[1], rotors[2], rotors[3]);
	strcat(buffer, temp);

	/* Send data */
	server_broadcast(server, buffer, strlen(buffer));
}

void process_control(int serial_fd, motion_context* motion, unsigned int* rotors, float time_total, float time_delta)
{
	float pitchOutput, pitchError, pitchSP, pitchPV;
	float rollOutput, rollError, rollSP, rollPV;
	unsigned char buffer[5];
	float fRotors[4];

	pitchSP = 0;
	pitchPV = motion->pitch;
	pitchError = pitchSP - pitchPV;
	pitchOutput = PID_PITCH_KP * pitchError;
	pitchOutput = fmax(fmin(pitchOutput, pitchOutput + PID_PITCH_MAX), pitchOutput - PID_PITCH_MAX);
	
	rollSP = 0;
	rollPV = motion->roll;
	rollError = rollSP - rollPV;
	rollOutput = PID_ROLL_KP * rollError;
	rollOutput = fmax(fmin(rollOutput, rollOutput + PID_ROLL_MAX), rollOutput - PID_ROLL_MAX);

	/* Rotors are mounted as follows:
	   R1   R2
	      O
	   R4   R3

	*/

	fRotors[0] = (SPEED_BASE - pitchOutput - rollOutput);
	fRotors[1] = (SPEED_BASE - pitchOutput + rollOutput);
	fRotors[2] = (SPEED_BASE + pitchOutput + rollOutput);
	fRotors[3] = (SPEED_BASE + pitchOutput - rollOutput);

	rotors[0] = (unsigned int)fmax(fmin(fRotors[0], SPEED_MAX), SPEED_MIN);
	rotors[1] = (unsigned int)fmax(fmin(fRotors[1], SPEED_MAX), SPEED_MIN);
	rotors[2] = (unsigned int)fmax(fmin(fRotors[2], SPEED_MAX), SPEED_MIN);
	rotors[3] = (unsigned int)fmax(fmin(fRotors[3], SPEED_MAX), SPEED_MIN);
	
	buffer[0] = (unsigned char)CMD_SETPWMCHANNELS;
	buffer[1] = (unsigned char)rotors[0];
	buffer[2] = (unsigned char)rotors[1];
	buffer[3] = (unsigned char)rotors[2];
	buffer[4] = (unsigned char)rotors[3];

	serial_write(serial_fd, (char*)buffer, 5);
}

