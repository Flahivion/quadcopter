#include "server.h"

static struct libwebsocket_protocols server_protocols[] =
{
	{
		"socket-server",
		server_socket_callback,
		sizeof(struct socket_server_session_data),
		128
	},
	{ NULL, NULL, 0, 0 }
};

int server_open(int port, server_context* context)
{
	struct libwebsocket_context* lws_context;
	struct lws_context_creation_info info;
	int opts = 0;
	
	memset(&info, 0, sizeof(struct lws_context_creation_info));
	
	info.port = port;
	info.protocols = server_protocols;
	info.gid = -1;
	info.uid = -1;
	info.options = opts;
	info.user = context;
	
	lws_context = libwebsocket_create_context(&info);
	if (lws_context == NULL)
		return -1;
	
	memset(context, 0, sizeof(server_context));
	context->lws_context = lws_context;
	context->send_buffer = NULL;
	context->send_length = 0;
	
	return 0;
}

void server_close(server_context* context)
{
	if (context != NULL)
	{
		if (context->lws_context != NULL)
		{
			libwebsocket_context_destroy(context->lws_context);
			context->lws_context = NULL;
			context->send_buffer = NULL;
			context->send_length = 0;
		}
	}
}

void server_process(server_context* context)
{
	libwebsocket_service(context->lws_context, 0);
}

void server_broadcast(server_context* context, char* buffer, int length)
{
	context->send_buffer = buffer;
	context->send_length = length;
	
	libwebsocket_callback_on_writable_all_protocol(&server_protocols[0]);
}

int server_socket_callback(
	struct libwebsocket_context* lws_context,
	struct libwebsocket* wsi,
	enum libwebsocket_callback_reasons reason,
	void* user,
	void* in,
	size_t len)
{
	char* buffer;
	int bytes_sent;
	server_context* context = (server_context*)libwebsocket_context_user(lws_context);

	switch (reason)
	{
		case LWS_CALLBACK_ESTABLISHED:
			printf("Client connection established.\n");
			break;
		
		case LWS_CALLBACK_PROTOCOL_DESTROY:
			printf("Client connection closed.\n");
			break;
			
		case LWS_CALLBACK_SERVER_WRITEABLE:
			if (context->send_buffer != NULL)
			{
				buffer = (char*)malloc(LWS_SEND_BUFFER_PRE_PADDING + context->send_length + LWS_SEND_BUFFER_POST_PADDING);
				memset(buffer, 0, LWS_SEND_BUFFER_PRE_PADDING + context->send_length + LWS_SEND_BUFFER_POST_PADDING);
				memcpy(buffer + LWS_SEND_BUFFER_PRE_PADDING, context->send_buffer, context->send_length);
				
				bytes_sent = libwebsocket_write(wsi, buffer + LWS_SEND_BUFFER_PRE_PADDING, context->send_length, LWS_WRITE_TEXT);
				free(buffer);
				
				if (bytes_sent < context->send_length)
				{
					printf("Error writing to client socket.\n");
					return -1;
				}
			}
			
			break;
			
		case LWS_CALLBACK_RECEIVE:
			printf("Received client data\n");
			break;
	}

	return 0;
}
