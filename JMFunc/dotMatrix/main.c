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

void main(int argc, char **argv)
{ 
	setDotMatrix(3);
	sleep(2);
	setDotMatrix(39);
	sleep(2);
	setDotMatrix(100);
	sleep(2);
	setDotMatrix(59);
	sleep(2);
	return;
}
