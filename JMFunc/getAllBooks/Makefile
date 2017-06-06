BDM: main.o getAllBooks.o
	gcc -pthread -o BDM \
		main.o getAllBooks.o

main.o:	main.c

getAllBooks.o:	getAllBooks.c getAllBooks.h

CC = gcc	

clean:
	rm -r *.o
