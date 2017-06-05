BDM: main.o keyMatrix.o
	arm-none-linux-gnueabi-gcc -pthread -o BDM \
		main.o keyMatrix.o

main.o:	main.c

keyMatrix.o:	keyMatrix.c keyMatrix.h

CC = /root/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc	

clean:
	rm -r *.o
