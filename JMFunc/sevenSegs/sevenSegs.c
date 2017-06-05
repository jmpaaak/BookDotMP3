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

#include "sevenSegs.h"


pthread_t threads[2];

int inactiveTIndex;
int endFlagSevenSegs;

void *sevenSegs(int value)
{ 
	int fd;

	fd = open("/dev/fnd",O_RDWR);
	if (fd<0)
	{
		perror("fnd driver open error.\n");
		return 1;
	}

	while(1 && endFlagSevenSegs == -1) {
		write(fd, &value, 4);
		usleep(100000);
	}

	close(fd);

	pthread_exit((void *) 0);
};

void initPage(int* page) {
	inactiveTIndex = 1;
	endFlagSevenSegs = -1;
	pthread_create(&threads[0], NULL, &sevenSegs, (void*) (*page));
};

void setNextPage(int* page) {
	// End Cur Page
	endFlagSevenSegs = 1;

	int status, rc;

	if(inactiveTIndex == 0)
		rc = pthread_join(threads[1], (void **) &status);
	else
		rc = pthread_join(threads[0], (void **) &status);	

	if(rc != 0)
		printf("Error thread_join() rc=%d\n ", rc);


	// Start Next Page
	endFlagSevenSegs = -1;
	*page = (*page)+1;
	pthread_create(&threads[inactiveTIndex], NULL, &sevenSegs, (void*) (*page));
	

	// convert inactiveTIndex
	if(inactiveTIndex == 0) 
		inactiveTIndex = 1;	
	else
		inactiveTIndex = 0;	
}

