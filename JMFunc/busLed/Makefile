BDM: main.o busLed.o
	arm-none-linux-gnueabi-gcc -pthread -o BDM \
		main.o busLed.o

main.o:	main.c

busLed.o:	busLed.c busLed.h

CC = /root/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc	

clean:
	rm -r *.o
