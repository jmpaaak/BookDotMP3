BDM: main.o colorLed.o
	arm-none-linux-gnueabi-gcc -pthread -o BDM \
		main.o colorLed.o

main.o:	main.c

colorLed.o:	colorLed.c colorLed.h

CC = /root/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc	

clean:
	rm -r *.o
