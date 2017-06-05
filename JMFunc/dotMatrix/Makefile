BDM: main.o dotMatrix.o
	arm-none-linux-gnueabi-gcc -pthread -o BDM \
		main.o dotMatrix.o

main.o:	main.c

dotMatrix.o:	dotMatrix.c dotMatrix.h

CC = /root/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc	

clean:
	rm -r *.o
