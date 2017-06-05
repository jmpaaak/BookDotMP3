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

void main(int argc, char **argv)
{ 
	int page = 0;

	initPage(&page);
	sleep(1);
	setNextPage(&page);
	sleep(1);
	setNextPage(&page);
	sleep(1);
	setNextPage(&page);
	sleep(1);
	setNextPage(&page);

	return;
}
