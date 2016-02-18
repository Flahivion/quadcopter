#include "serial.h"

int serial_open(const char* filename)
{
	int fd;
	struct termios tty;

	fd = open(filename, O_RDWR | O_NOCTTY);
	if (fd < 0)
	{
		printf("Error %d opening %s: %s\n", errno, filename, strerror(errno));
		return -1;
	}

	memset(&tty, 0, sizeof(tty));
	if (tcgetattr(fd, &tty) != 0)
	{
		printf("Error %d retrieving tty attributes\n", errno);
		return -1;
	}

	cfmakeraw(&tty);

	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 0;
	tty.c_cflag &= ~(CSIZE);
	tty.c_cflag |= (PARENB | PARODD | CS8 | HUPCL); // Enable odd parity
	tty.c_cflag &= ~CSTOPB; // One stop bit
	tty.c_cflag &= ~CRTSCTS; // Disable hardware flow control
	tty.c_oflag = 0;
	tty.c_iflag &= ~(IXON);
	tty.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

	if (cfsetospeed(&tty, B115200) != 0)
	{
		printf("Error %d setting output baud rate\n", errno);
		return -1;
	}

	if (cfsetispeed(&tty, B115200) != 0)
	{
		printf("Error %d setting input baud rate\n", errno);
		return -1;
	}

	if (tcsetattr(fd, TCSANOW, &tty) != 0)
	{
		printf("Error %d setting tty attributes\n", errno);
		return -1;
	}

	return fd;
}

int serial_close(int fd)
{
	if (fd < 0)
		return -1;

	return close(fd);
}

int serial_write(int fd, const char* buffer, unsigned int length)
{
	if (fd < 0)
		return -1;

	return write(fd, buffer, length);
}

