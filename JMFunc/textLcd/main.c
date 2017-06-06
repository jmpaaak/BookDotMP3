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

#include "tLcd.h"

void main(int c, char **v)
{ 
	char arg[15] = "Hi!, Book.MP3";
	loadTextTLcd(arg);
	sleep(2);
 	char arg2[15] = "Hi!, BP3";
	loadTextTLcd(arg2);
	sleep(2);
	char arg3[15] = "Hi!, o3";
	loadTextTLcd(arg3);
	sleep(2);
	return;
}
