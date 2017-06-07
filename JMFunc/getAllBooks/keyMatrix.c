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

#include "keyMatrix.h"

pthread_t thread;

int endFlag;

extern int callback(int curKey);

void *keyMatrix(void)
{ 
	int fd;
	int retvalue = 0;
	
	fd = open("/dev/kmat",O_RDWR);
	if (fd<0)
	{
		perror("keymatrix driver open error.\n");
		return 1;
	}

	while(endFlag == -1) {
		read(fd, &retvalue, 4);
		if(retvalue != 0)
			callback(retvalue);	
		sleep(1);
	}

	close(fd);
	return retvalue;

}

void initKeyMatrix()
{
	int status;
	endFlag = -1;
	pthread_create(&thread, NULL, &keyMatrix, (void*) status);
}
