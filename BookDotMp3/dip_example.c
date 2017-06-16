// dipswitch.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "dipswitch.h"

int main() {

	while(1)
	{
		printf("dipswitch value: %0X\n", \
			dipswitch_read());
		sleep(1);
	}
}

int dipswitch_read()
{
	int fd;
	int retvalue;

	fd = open("/dev/dipswitch", O_RDWR);
	if( fd<0)
	{
		perror("driver open error\n");
		return 1;
	}

	read(fd,&retvalue,8);
	retvalue &= 0xff;

	close(fd);

	return retvalue;
}
