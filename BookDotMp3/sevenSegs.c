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

pthread_t seven_threads[2];
extern pthread_mutex_t mutex;

int seven_inactiveTIndex;
int seven_endFlagSevenSegs;

void *sevenSegs(int value)
{ 
	int fd;

	fd = open("/dev/fnd",O_RDWR);
	if (fd<0)
	{
		perror("fnd driver open error.\n");
		return 1;
	}

	while(1 && seven_endFlagSevenSegs == -1) {
		write(fd, &value, 4);
		usleep(100000);
	}
	close(fd);

	pthread_exit((void *) 0);
};

void initPage(int* page) {
	seven_inactiveTIndex = 1;
	seven_endFlagSevenSegs = -1;
	pthread_create(&seven_threads[0], NULL, &sevenSegs, (void*) (*page));
};

void setNextPage(int* page) {
	// End Cur Page
	seven_endFlagSevenSegs = 1;

	int status, rc;

	if(seven_inactiveTIndex == 0)
		rc = pthread_join(seven_threads[1], (void **) &status);
	else
		rc = pthread_join(seven_threads[0], (void **) &status);	

	if(rc != 0)
		printf("Error thread_join() rc=%d\n ", rc);


	// Start Next Page
	seven_endFlagSevenSegs = -1;
	*page = (*page)+1;
	pthread_create(&seven_threads[seven_inactiveTIndex], NULL, &sevenSegs, (void*) (*page));
	

	// convert inactiveTIndex
	if(seven_inactiveTIndex == 0) 
		seven_inactiveTIndex = 1;	
	else
		seven_inactiveTIndex = 0;	
}

