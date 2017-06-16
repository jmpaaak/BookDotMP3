#ifndef __O_LED__
void initOled();
void doHelp(void);
unsigned long simple_stroul(char * cp, char **endp, unsigned int base);
unsigned long read_hex(const char* str);
int reset(void);
int writeCmd(int size, unsigned short* cmdArr);
int writeData(int size, unsigned char* dataArr);
int readData(int size, unsigned short* dataArr);
int setAddrssDefalut(void);
int setCmdLock(int block);
int imageLoading(char* filename);
int Init(void);
int setOLed(int argc, char **argv);
void loadImgOLed(char * imgPath);
#endif
