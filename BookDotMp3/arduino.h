// arduino.h
#define BAUDRATE 9600
#define MODEMDEVICE "/dev/ttyACM0"
#define _POSIX_SOURCE 1 /* POSIX compiant source */
#define FALSE 0
#define TRUE 1

void arduino_serial_write(char* command);
int arduino_serial_read();
