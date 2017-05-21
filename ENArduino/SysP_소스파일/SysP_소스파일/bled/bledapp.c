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
	int fd_dipsw;
	int fd_mled;
	int retvalue;
	int i=0;
	
	fd_dipsw = open("/dev/dipsw",O_RDWR);
	fd_mled = open("/dev/bled",O_RDWR);
	if (fd<0)
	{
		perror("driver open error.\n");
		return 1;
	}
	while(1)
	{	
		read(fd_dipsw,&retvalue,1);
		retvalue &= 0xFF;
		printf("dipsw state : %X\n", retvalue);
		write(fd_mled, &dipsw_value, 2);
	}
	close(fd);
	return retvalue;

}
