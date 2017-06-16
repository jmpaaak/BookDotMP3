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

#include "dotmatrix.h"

extern pthread_mutex_t mutex;

pthread_t dot_threads[2];

int dot_inactiveTIndex = -1; // for auto init
int dot_endFlagDotMatrix;

void *dotMatrix(unsigned short value)
{ 
	int fd_mled;

	fd_mled = open("/dev/mled",O_RDWR);

	if (fd_mled < 0)
	{
		perror("mled driver open error.\n");
		return 1;
	}

	while(dot_endFlagDotMatrix == -1) {
		write(fd_mled, &value, 2);
	}
	
	close(fd_mled);

	pthread_exit((void*) 0);
}

void setDotMatrix(unsigned short value) {
	if(dot_inactiveTIndex < 0) // init
	{
		dot_inactiveTIndex = 1;
		dot_endFlagDotMatrix = -1;
		pthread_create(&dot_threads[0], NULL, &dotMatrix, (void*) value);
	}
	else 
	{
		// End cur dotMatrix
		dot_endFlagDotMatrix = 1;

		int rc, res;

		if(dot_inactiveTIndex == 0)
			rc = pthread_join(dot_threads[1], (void**) &res);
		else
			rc = pthread_join(dot_threads[0], (void**) &res);

		if(rc != 0)
			printf("Error thread_join() rc=%d\n", rc);


		// Start Next dotMatrix value
		dot_endFlagDotMatrix = -1;
		pthread_create(&dot_threads[dot_inactiveTIndex], NULL, &dotMatrix, (void*) value);


		// convert inactiveTIndex
		if(dot_inactiveTIndex == 0)
			dot_inactiveTIndex = 1;
		else
			dot_inactiveTIndex = 0;
	}
}
