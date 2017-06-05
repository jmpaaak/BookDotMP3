BDM: main.o sevenSegs.o
		arm-none-linux-gnueabi-gcc -pthread -o BDM \
			main.o sevenSegs.o

main.o: main.c

sevenSegs.o: sevenSegs.c sevenSegs.h

CC = /root/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc

clean:
	rm -r *.o
