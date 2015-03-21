#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include "motion.h"

int server_start(const char* url, int* out_socket);
void server_stop(int socket);
int server_send(int socket, const char* data, int length);

int main(int argc, char* argv[])
{
	int server_socket;
	char data[512];
	char temp[128];
	int bytes_sent;
	int bytes_to_send;
	float accel[3], gyro[3], compass[3];
	int result;
	const char* device = "/dev/i2c-1";
	
	if (argc != 2)
	{
		printf("Usage: mpu <server_address>\n");
		return EXIT_FAILURE;
	}
	
	if (motion_init(device) != 0)
	{
		printf("Failed to initialize motion processing unit.\n");

		motion_close();
		return EXIT_FAILURE;
	}

	if (server_start(argv[1], &server_socket) != 0)
	{
		printf("Failed to initialize server on address %s\n", argv[1]);
		motion_close();
		return EXIT_FAILURE;
	}
	
	sleep(1);

	while (1)
	{
		result = motion_sample(accel, gyro, compass);
		data[0] = '\0';
		
		if (result & SAMPLE_ACCEL)
		{
			sprintf(temp, "AX:%7.4f,AY:%7.4f,AZ:%7.4f;", accel[0], accel[1], accel[2]);
			strcat(data, temp);
		}
		
		if (result & SAMPLE_GYRO)
		{
			sprintf(temp, "GX:%7.4f,GY:%7.4f,GZ:%7.4f;", gyro[0], gyro[1], gyro[2]);
			strcat(data, temp);
		}
		
		if (result & SAMPLE_COMPASS)
		{
			sprintf(temp, "CX:%7.4f,CY:%7.4f,CZ:%7.4f;", compass[0], compass[1], compass[2]);
			strcat(data, temp);
		}
		
		printf("Data to send: %s\n", data);
		bytes_to_send = strlen(data) + 1;
		bytes_sent = server_send(server_socket, data, bytes_to_send);
		if (bytes_sent != bytes_to_send)
		{
			printf("Failed to send data: %d of %d bytes sent.\n", bytes_sent, bytes_to_send);
		}
	}

	server_stop(server_socket);
	motion_close();
	
	return EXIT_SUCCESS;
}

int server_start(const char* url, int* out_socket)
{
	*out_socket = nn_socket(AF_SP, NN_PUB);
	if (*out_socket < 0)
		return 1;
	
	if (nn_bind(*out_socket, url) < 0)
	{
		nn_shutdown(*out_socket, 0);
		return 1;
	}
	
	return 0;
}

void server_stop(int socket)
{
	nn_shutdown(socket, 0);
}

int server_send(int socket, const char* data, int length)
{
	int bytes_sent;
	
	bytes_sent = nn_send(socket, data, length, 0);
	
	return bytes_sent;
}

