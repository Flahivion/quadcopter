#ifndef _SERVER_H_
#define _SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libwebsockets.h>

struct socket_server_session_data
{
	struct libwebsocket* wsi;
};

typedef struct
{
	struct libwebsocket_context* lws_context;
	char* send_buffer;
	int send_length;
} server_context;

int server_open(int port, server_context* context);
void server_close(server_context* context);
void server_process(server_context* context);
void server_broadcast(server_context* context, char* buffer, int length);

int server_socket_callback(
	struct libwebsocket_context* context,
	struct libwebsocket* wsi,
	enum libwebsocket_callback_reasons reason,
	void* user,
	void* in,
	size_t len);


#endif
