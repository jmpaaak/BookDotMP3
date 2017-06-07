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


char** getAllBooks()
{
	int size = sizeof(char*) * 999;
	char **books = (char **) malloc( size );

	int i;
	for(i=0; i<999; i++)
		books[i] = (char *) malloc( 15 );

	// max book num 999
	int numOfBooks = 0;
	system("wget -O index.html http://54.251.159.248/BookDotMP3/");
	FILE * pIndex = fopen("index.html", "r");
	char line[1000];
	char * pTarget;

	while(!feof(pIndex)) {
		fgets(line, 1000, pIndex);
		if ((pTarget = strstr(line, "[DIR]")) != NULL) {
			char * temp = strstr(pTarget, "href=\"");
			char bookName[15];
			int i = 0;

			temp += 6;
 
			while((*temp) != '/') {
				bookName[i] = (*temp);
				i++;
				temp++;
			}
			bookName[i] = '\0'; 

			// add to list
			strcpy(books[numOfBooks], bookName);
			numOfBooks++;
		}
	}
	
	close(pIndex);
	return books;
}
