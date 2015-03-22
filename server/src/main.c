#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "motion.h"
#include "server.h"

#define SEND_BUFFER_LENGTH 1024

void handle_sigint(int signal);
void process_data(
	server_context* server,
	motion_context* motion,
	float time_total,
	float time_delta,
	char* buffer,
	int length);

static int _force_exit = 0;

int main(int argc, char* argv[])
{
	server_context server;
	char* send_buffer;
	struct timespec prev_time, cur_time;
	motion_context motion;
	float time_total;
	float time_delta;

	if (argc != 3)
	{
		printf("Usage: motioncontrold <i2c_address> <websocket_port>\n");
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
		
		process_data(&server, &motion, time_total, time_delta, send_buffer, SEND_BUFFER_LENGTH);
		server_process(&server);
	}

	server_close(&server);
	motion_close();
	
	free(send_buffer);
	send_buffer = NULL;
	
	return EXIT_SUCCESS;
}

void handle_sigint(int signal)
{
	printf("SIGINT received, exiting...\n");
	_force_exit = 1;
}

void process_data(server_context* server, motion_context* motion, float time_total, float time_delta, char* buffer, int length)
{
	char temp[128];
	
	/* Process motion data */
	motion_process(motion, time_delta);
	
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
	
	/* Send data */
	server_broadcast(server, buffer, strlen(buffer));
}
