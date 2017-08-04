#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include "serial.h"

/*
 * 'serial_open()' - Open serial port 
 *
 * Returns the file descriptor on success or -1 on error.
 */

int serial_open(char * device_name)
{
	int fd; /* File descriptor for the port */
	struct termios options;

	fd = open(device_name, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1){
		return -1;
	}
	else{
		fcntl(fd, F_SETFL, 0);
	}

	/*
	 * Get the current options for the port...
	 */
	tcgetattr(fd, &options);

	/*
	 * Set the baud rates to 9600
	 */
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);

	/*
	 * Enable the receiver and set local mode...
	 */
	options.c_cflag |= (CLOCAL | CREAD);

	/* 8N1 */
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	/* disable hardware flow control */
	options.c_cflag &= ~CRTSCTS;
	/* disable software flow control */
	options.c_iflag &= ~(IXON | IXOFF | IXANY);

	/* raw input */
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	/* block wait with timeout of 5s, need time for programming the flash memory */
	options.c_cc[VTIME] = 50; /* timeout = VTIME * 0.1 inter-character */
	options.c_cc[VMIN] = 0;   /* blocking read until 1 chars received */

	/* raw output */
	options.c_oflag &= ~OPOST;

	/*
	 * Set the new options for the port...
	 */
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &options);

	/* non blocking read */
	//fcntl(fd, F_SETFL, FNDELAY);

	return (fd);
}

void serial_close(int fd)
{
	close(fd);
	return;
}

/* read */
int serial_read_data(int fd, char *pchar, int size){
	int n;
	int rc;

	for (n=0; n<size; n++){
		rc = read(fd, pchar+n, 1);
		if (rc!=1){
			return n;
		}
	}
	return size;
}

/* write data and wait for echo */
int serial_write_read_data(int fd, char *pchar, int size){
	char echo;
	int rc;
	int n;

	for (n=0; n<size; n++){
		/* write data */
		write(fd, &pchar[n], 1);
		/* read data back */
		rc = read(fd, &echo, 1); 
		if (rc > 0){
			/* read back check */	
			if (pchar[n] != echo){
				printf("\n%s : read back error at index %d of %d, sent %02x, received %02x\n", 
							__FUNCTION__, n, size-1,
							(unsigned char)pchar[n], echo);
				return -1;
        	}
		}
    	else{
			printf("\n%s : timeout, sent %d of %d, %02x\n",
				__FUNCTION__, n, size-1, pchar[n]);
			return -1;
    	}	
	}
	return size;
}

void dump_data(char *pdata, int size)
{
	int n;
	for(n=0;n<size;n++){
		if ((n%16) ==0){
			printf("\n");
		}
		printf("%02x ", pdata[n] & 0xff);
	}
	printf("\n");
}

int example_main(int argc, char **argv)
{
	int fd;
	char * device_name = "/dev/ttyWCH0";
	unsigned char packet[256];
	int n;
	
	fd = serial_open(device_name);
	if (fd == -1){
		printf("Unable to open %s ", device_name);
		return 0;
	}

	for(n=0; n<500; n++){
		if (read(fd, packet, 1) > 0){
			printf("%c", packet[0]);
			packet[0] = '$';
			fflush(stdout);
		}
		else{
			printf("\ttimeout\n");
		}
	}
	
	serial_close(fd);
	return 0;
}
