BDM: main.o oLed.o
	arm-none-linux-gnueabi-gcc -pthread -o BDM \
		main.o oLed.o

main.o:	main.c

oLed.o:	oLed.c oLed.h

CC = /root/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc	

clean:
	rm -r *.o
