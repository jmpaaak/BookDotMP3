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

void setBusLedNum(char num)
{
    int	fd = open("/dev/bled",O_RDWR);
	if (fd<0)
	{
		perror("bled driver open error.\n");
		return 1;
	}

	write(fd, &num, 1);

	close(fd);

}
