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

#include "oLed.h"

void main(int c, char **v)
{ 
	loadImgOLed("oled_test.jpg.img");
	sleep(100);
	return;
}
