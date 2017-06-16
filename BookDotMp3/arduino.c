// arduino.c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arduino.h"

void arduino_serial_write(char* command) {
	int STATE_OK=0;
    	int STATE_WARNING=1;
    	int STATE_CRITICAL=2; 
    	char tempbuf[10];
    	struct termios tty;

    	int fd=open("/dev/ttyACM0",O_RDWR | O_NOCTTY);
    	if(fd == -1){
            	printf("Unable to open /dev/ttyACM0\n");
           	return;
    	}
	else 
	{
        	if(tcgetattr(fd, &tty)!=0)
		{
            		perror("tcgetatt() error");
        	}
		else
		{
                	cfsetospeed(&tty, B9600);
                	cfsetispeed(&tty, B9600);

                	tty.c_cflag &= ~PARENB;
                	tty.c_cflag &= ~CSTOPB;
                	tty.c_cflag &= ~CSIZE;
                	tty.c_cflag |= CS8;
                	tty.c_cflag &= ~CRTSCTS; 
                	tty.c_cflag |= CLOCAL | CREAD;

                	tty.c_iflag |= IGNPAR | IGNCR;
                	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
                	tty.c_lflag |= ICANON;
                	tty.c_oflag &= ~OPOST;
                	tcsetattr(fd, TCSANOW, &tty);

			sleep(1);

                	int w=write(fd, command, 1);
        	}
    	}
    	
	close(fd);

	return;	
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
	//system(close_command);
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
