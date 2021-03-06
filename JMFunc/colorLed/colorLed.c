#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "colorLed.h"

pthread_t threads[2];

int inactiveTIndex = -1; // for automatic init
int endFlagColorLed;

void *colorLedThread(char *status)
{
    int	fd = open("/dev/cled",O_RDWR);
	if (fd<0)
	{
		perror("cled driver open error.\n");
		return 1;
	}

	// r:1~4, g:5~8, b:9~12
	unsigned char i=0;
	int seq=1; // 1: CW, -1:CCW
	char rOff=1, gOff=5, bOff=9;

	// first clear
	write(fd, &rOff, 1);
	write(fd, &gOff, 1);
	write(fd, &bOff, 1);

	if(strcmp(status, "wait") == 0) 
	{
		while(endFlagColorLed == -1) {
			i = 2;
			write(fd, &rOff, 1);
			write(fd, &gOff, 1);
			write(fd, &bOff, 1);
			usleep(550000);
			write(fd, &i, 1);
			i += 4;
			write(fd, &i, 1);
			i += 4;
			write(fd, &i, 1);
			usleep(550000);
		}
	}
	else if(strcmp(status, "process") == 0) {
		i = 4;
		while(endFlagColorLed == -1) 
		{
			if(i == 12) seq = -1;	
			else if(i == 4) seq = 1;
			write(fd, &i, 1);
			
			// off
			if(i == 4) { 		//r
				write(fd, &gOff, 1);
				write(fd, &bOff, 1);
			}
			else if(i == 8) {	//g
				write(fd, &rOff, 1);
				write(fd, &bOff, 1);
			}
			else if(i == 12) {	//b
				write(fd, &rOff, 1);
				write(fd, &gOff, 1);
			}

			i += 4*seq;
			usleep(350000); // 0.35s
		}
	}
	else if(strcmp(status, "read") == 0) {
		i = 5;
		while(endFlagColorLed == -1) 
		{
			if(i == 8) seq = -1;	
			else if(i == 5) seq = 1;
			write(fd, &i, 1);
			i += seq;
			usleep(350000); // 0.35s
		}
	}

	close(fd);

	pthread_exit((void*) 0); 
}

void setColorLed(char *status)
{ 
	if(inactiveTIndex < 0) // init
   	{
		inactiveTIndex = 1;
		endFlagColorLed = -1;
		pthread_create(&threads[0], NULL, &colorLedThread, (void*) status);
	}
	else
	{
		// End Cur colorLed
		endFlagColorLed = 1;

		int rc, res;

		if(inactiveTIndex == 0)
			rc = pthread_join(threads[1], (void **) &res);
		else	
			rc = pthread_join(threads[0], (void **) &res);

		if(rc != 0)
			printf("Error thread_join() rc=%d\n", rc);
		

		// Start Next colorLed status
		endFlagColorLed = -1;
		pthread_create(&threads[inactiveTIndex], NULL, &colorLedThread, (void*) status);


		// convert inactiveTIndex
		if(inactiveTIndex == 0)
			inactiveTIndex = 1;
		else
			inactiveTIndex = 0;
	}
};
