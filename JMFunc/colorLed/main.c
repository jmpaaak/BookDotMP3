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

void main(int argc, char **argv)
{ 
	setColorLed("process");
	sleep(5);
	setColorLed("wait");
	sleep(5);
	setColorLed("read");
	sleep(5);
	return;
}
