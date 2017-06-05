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

#include "busLed.h"

void main(int argc, char **argv)
{ 
	setBusLedNum(5);
	sleep(1);
	setBusLedNum(3);
	sleep(1);
	setBusLedNum(8);
	sleep(1);
	setBusLedNum(0);
	sleep(1);
	setBusLedNum(6);
	sleep(1);
	return;
}
