// arduino.c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arduino.h"

int main(void) {

	char command[10] = "T";
	
	arduino_serial_write(command);

	return 0;
}

void arduino_serial_write(char* command) {
	/*
	int fd, c, res;
	struct termios oldtio, newtio;
	char buf[255];

	fd= open(MODEMDEVICE, O_RDWR | O_NOCTTY);
	if(fd <0) { perror(MODEMDEVICE); exit(-1); }
	
	tcgetattr(fd, &oldtio);

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CRTSCTS | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	// set input mode 
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 5;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	printf("command: %s\n", command);
	res = write(fd, command, 20);
	printf("res: %d\n", res);

	tcsetattr(fd, TCSANOW, &oldtio);
	*/

	char system_command_first[200];

	sprintf(system_command_first, "busybox microcom -t 9600 %s", \
			MODEMDEVICE);
	system(system_command_first);

	system(command);
	system(command);
	system(command);

	system("^X");
}

int arduino_serial_read() { 
	int fd, c, res;
	struct termios oldtio, newtio;
	char buf[255];

	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY);
	if(fd <0) { perror(MODEMDEVICE); exit(-1); }

	tcgetattr(fd, &oldtio); /* save current port settings */
	
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CRTSCTS | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo, ...) */
	newtio.c_lflag = 0;
	
	newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
	newtio.c_cc[VMIN] = 1; /* blocking read until 1 chars received */

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	/* loop for input */
	res = read(fd, buf, 255); 
	/* returns after * chars have been input */
	buf[res] = 0; /* so we can printf... */

	tcsetattr(fd, TCSANOW, &oldtio);

	if(strcmp("D", buf)==0) {
		return 0;
	}
	else {
		return 1;
	}
}
