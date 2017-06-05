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

#include "dotMatrix.h"

pthread_t threads[2];

int inactiveTIndex = -1; // for auto init
int endFlagDotMatrix;

void *dotMatrix(unsigned short value)
{ 
	int fd_mled;

	fd_mled = open("/dev/mled",O_RDWR);

	if (fd_mled < 0)
	{
		perror("mled driver open error.\n");
		return 1;
	}

	while(endFlagDotMatrix == -1) {
		write(fd_mled, &value, 2);
	}

	close(fd_mled);

	pthread_exit((void*) 0);
}

void setDotMatrix(unsigned short value) {
	if(inactiveTIndex < 0) // init
	{
		inactiveTIndex = 1;
		endFlagDotMatrix = -1;
		pthread_create(&threads[0], NULL, &dotMatrix, (void*) value);
	}
	else 
	{
		// End cur dotMatrix
		endFlagDotMatrix = 1;

		int rc, res;

		if(inactiveTIndex == 0)
			rc = pthread_join(threads[1], (void**) &res);
		else
			rc = pthread_join(threads[0], (void**) &res);

		if(rc != 0)
			printf("Error thread_join() rc=%d\n", rc);


		// Start Next dotMatrix value
		endFlagDotMatrix = -1;
		pthread_create(&threads[inactiveTIndex], NULL, &dotMatrix, (void*) value);


		// convert inactiveTIndex
		if(inactiveTIndex == 0)
			inactiveTIndex = 1;
		else
			inactiveTIndex = 0;
	}
}
