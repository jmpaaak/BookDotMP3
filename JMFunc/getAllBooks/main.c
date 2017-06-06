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

#include "getAllBooks.h"

void main(int c, char **v)
{ 
	char **books;
	books =	getAllBooks(); 
	int j,i;
	for(i=0; i<sizeof(books); i++) 
	{ //TODO error occurs
		printf("book : ");
		for(j=0; j<sizeof(books[i]); j++) 
			printf("%c", books[i][j]);
	}
	return;
}
