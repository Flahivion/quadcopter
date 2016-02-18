#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

int serial_open(const char* filename);
int serial_close(int fd);
int serial_write(int fd, const char* buffer, unsigned int length);

#endif
