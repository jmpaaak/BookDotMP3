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
#include "oLed.h"
#include "tLcd.h"
#include "keyMatrix.h"

char **books;
int curDirIndex = 0;
int maxNumBooks = 0;

void callback(int curKey)
{
	switch(curKey) {
		case 5:
			if(curDirIndex == 0) {
				curDirIndex = maxNumBooks;
			} else {
				curDirIndex--;
			}
			break;
		case 7:
			if(curDirIndex == maxNumBooks) {
				curDirIndex = 0;
			} else {
				curDirIndex++;
			}
			break;
	}

	char * coverName = "/cover.jpg.img";
	char * baseCommand = "wget -O cover.jpg.img http://54.251.159.248/BookDotMP3/";
	strcat(baseCommand, books[curDirIndex]);
	strcat(baseCommand, coverName);
	system(baseCommand); 			// get cover.jpg.img
	loadImgOLed("./cover.jpg.img"); // set cover.jpg.img
	loadTextTLcd(books[curDirIndex]); 			// set book name 

}

void main(int c, char **v)
{ 
	books =	getAllBooks();

	curDirIndex = 0;
	maxNumBooks = sizeof(books)/4; 

	initKeyMatrix();

	char * coverName = "/cover.jpg.img";
	char * baseCommand = "wget -O cover.jpg.img http://54.251.159.248/BookDotMP3/";
	strcat(baseCommand, books[curDirIndex]);
	strcat(baseCommand, coverName);
	system(baseCommand); 				// get cover.jpg.img
	loadImgOLed("./cover.jpg.img"); 	// set cover.jpg.img
	loadTextTLcd(books[curDirIndex]); 	// set book name 
	
	/* find get all books
	for(i=0; i<sizeof(books)/4; i++) 
	{ 	
		printf("\n\nbook %d: ", i);
		for(j=0; j<sizeof(books[i]); j++)
			printf("%c", books[i][j]);

		printf("\n\n");
	}
	*/
	return;
}
