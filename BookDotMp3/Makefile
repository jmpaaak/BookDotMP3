
bookDotMp3: main.o camera.o arduino.o speech.o vision.o play.o \
	    dipswitch.o sevenSegs.o dotmatrix.o colorLed.o busLed.o \
	    textLcd.o oLed.o keyMatrix.o getAllBooks.o
	arm-none-linux-gnueabi-gcc -o bookDotMp3 \
	main.o camera.o arduino.o speech.o vision.o play.o \
	dipswitch.o sevenSegs.o dotmatrix.o colorLed.o busLed.o  \
	textLcd.o oLed.o keyMatrix.o getAllBooks.o \
	-lssl -lcrypto -lpthread

main.o: main.c main.h

camera.o: camera.c camera.h

arduino.o: arduino.c arduino.h

speech.o: speech.c speech.h

vision.o: vision.c vision.h

play.o: play.c play.h

dipswitch.o: dipswitch.c dipswitch.h

sevenSegs.o: sevenSegs.c sevenSegs.h

dotmatrix.o: dotmatrix.c dotmatrix.h

colorLed.o: colorLed.c colorLed.h

busLed.o: busLed.c busLed.h

textLcd.o: textLcd.c textLcd.h

oLed.o: oLed.c oLed.h

keyMatrix.o: keyMatrix.c keyMatrix.h

getAllBooks.o: getAllBooks.c getAllBooks.h

CC = /root/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc

LIBS = -lssl -lcrypto -lpthread

clean:
	rm -f *.o
