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

int main(int argc, char **argv)
{ 
	int fd;
	int value;
	
	value = 3406;

	fd = open("/dev/fnd",O_RDWR);
	if (fd<0)
	{
		perror("driver open error.\n");
		return 1;
	}

	while(1) {
		write(fd, &value, 4);
		sleep(0.1);
	}
	close(fd);
	return 1;
}
