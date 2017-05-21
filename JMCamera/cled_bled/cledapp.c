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
	int fd, fd_km, fd_bled;
	int value;
	
	fd = open("/dev/cled",O_RDWR);
	fd_km = open("/dev/keymatrix",O_RDWR);
	fd_bled = open("/dev/bled",O_RDWR);
	if (fd<0 || fd_km<0 || fd_bled<0)
	{
		perror("driver open error.\n");
		return 1;
	}

	unsigned char bval;
	while(1) {
		read(fd_km, &value, 4);
		printf("App: val %d\n", value);
		write(fd, &value, 4);

		write(fd_bled, &value, 1);
	}
	close(fd);
	close(fd_km);
	close(fd_bled);

	return 1;

}
