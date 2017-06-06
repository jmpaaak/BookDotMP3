BDM: main.o tLcd.o
	arm-none-linux-gnueabi-gcc -pthread -o BDM \
		main.o tLcd.o

main.o:	main.c

tLcd.o:	tLcd.c tLcd.h

CC = /root/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc	

clean:
	rm -r *.o
