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
	int retvalue;
	int i=0;
	
	fd = open("/dev/FND",O_RDWR);
	if (fd<0)
	{
		perror("driver open error.\n");
		return 1;
	}

	//unsigned short temp[6]={0xfe2d,0xfd4b,0xfb06,0xf706,0xef4f,0xdf4b};//521132
	unsigned short zero = 0xfe3f;	
	while(1) {
		write(fd, &zero, 2);
		//i++;
		sleep(100);
	}

	close(fd);
	return retvalue;

}
