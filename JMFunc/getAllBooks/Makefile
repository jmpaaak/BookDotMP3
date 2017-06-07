BDM: main.o getAllBooks.o oLed.o tLcd.o keyMatrix.o
	gcc -pthread -o BDM \
		main.o getAllBooks.o oLed.o tLcd.o keyMatrix.o

main.o:	main.c

oLed.o: oLed.c oLed.h

tLcd.o: tLcd.c oLed.h

keyMatrix.o: keyMatrix.c keyMatrix.h

getAllBooks.o:	getAllBooks.c getAllBooks.h

CC = gcc	

clean:
	rm -r *.o
