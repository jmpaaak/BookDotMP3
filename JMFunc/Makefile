BDM: main.o callGoogleVision.o
		arm-none-linux-gnueabi-gcc -o BDM \
			main.o callGoogleVision.o

main.o: main.c

callGoogleVision.o: callGoogleVision.c callGoogleVision.h

CC = /root/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc

clean:
	rm -r *.o
