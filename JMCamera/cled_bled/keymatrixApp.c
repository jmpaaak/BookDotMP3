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
	
	fd = open("/dev/keymatrix",O_RDWR);
	if (fd<0)
	{
		perror("driver open error.\n");
		return 1;
	}

	while(1) {
		read(fd, &retvalue, 4);
		if(retvalue != 0)
			printf("keymatrix state : %d\n", retvalue);		
		sleep(1);
	} 


	close(fd);
	return retvalue;

}
