#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define TRUE	1
#define FALSE	0

#define DRIVER_NAME		"/dev/cnoled"

static  int  fd ; 

void doHelp(void)
{
	printf("Usage:\n");
	printf("oledtest w d1 [d2] .... :=> write data \n");
	printf("oledtest r readnum  :=> read data \n");
	printf("oledtest c cmd [sub1] [sub2] .... :cmd set\n");
	printf("oledtest t :=> reset \n");
	printf("oledtest i :=> init \n");
	printf("oledtest d file(.img):=> loading image file\n");
}

unsigned long simple_strtoul(char *cp, char **endp,unsigned int base)
{
	unsigned long result = 0,value;
	
	if (*cp == '0') {
		cp++;
		if ((*cp == 'x') && isxdigit(cp[1])) {
			base = 16;
			cp++;
		}
		if (!base) {
			base = 8;
		}
	}
	if (!base) {
		base = 10;
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
								? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}

unsigned long read_hex(const char* str){
	char addr[128];
	strcpy(addr,str);
	return simple_strtoul(addr, NULL, 16);
}


// signal form 
//	12bit	11bit	10bit	9bit	8bit	7bit	6bit	5bit	4bit	3bit	2bit	1bit	0bit
//	RST#	CS#		D/C#	WD#		RD#		D7		D6		D5		D4		D3		D2		D1		D0
// trigger => WD or RD rising edge
/************************************************************************************************





************************************************************************************************/
#define RST_BIT_MASK	0xEFFF		
#define CS_BIT_MASK		0xF7FF
#define DC_BIT_MASK		0xFBFF
#define WD_BIT_MASK		0xFDFF
#define RD_BIT_MASK		0xFEFF
#define DEFAULT_MASK	0xFFFF


#define CMD_SET_COLUMN_ADDR		0x15
#define CMD_SET_ROW_ADDR		0x75
#define CMD_WRITE_RAM			0x5C
#define CMD_READ_RAM			0x5D
#define CMD_LOCK				0xFD

int reset(void)
{
	unsigned short wdata ;

	wdata = RST_BIT_MASK;
	write(fd,&wdata , 2 );
	usleep(2000);
	wdata = DEFAULT_MASK;
	write(fd,&wdata , 2 );
	return TRUE;
}

int writeCmd(int size , unsigned short* cmdArr)
{
	int i ;
	unsigned short wdata;

	//printf("wCmd : [0x%02X]",cmdArr[0]);
	//wdata = CS_BIT_MASK;
	//write(fd,&wdata,2);

	wdata = CS_BIT_MASK & DC_BIT_MASK;
	write(fd,&wdata,2);

	wdata = CS_BIT_MASK & DC_BIT_MASK & WD_BIT_MASK ;
	write(fd,&wdata,2);

	wdata = CS_BIT_MASK & DC_BIT_MASK & WD_BIT_MASK & (cmdArr[0]|0xFF00) ;
	write(fd,&wdata,2);
	
	wdata = CS_BIT_MASK & DC_BIT_MASK & (cmdArr[0] | 0xFF00) ;
	write(fd,&wdata,2);

	wdata = CS_BIT_MASK & ( cmdArr[0] | 0xFF00);
	write(fd,&wdata,2);

	for (i = 1; i < size ; i++ )
	{
	//	wdata = CS_BIT_MASK ;
	//	write(fd,&wdata,2);

	//	wdata = CS_BIT_MASK ;
	//	write(fd,&wdata,2);

		wdata = CS_BIT_MASK & WD_BIT_MASK ;
		write(fd,&wdata,2);

		wdata = CS_BIT_MASK & WD_BIT_MASK & (cmdArr[i] | 0xFF00) ;
		write(fd,&wdata,2);

		wdata = CS_BIT_MASK & (cmdArr[i] | 0xFF00);
		write(fd,&wdata,2);

	//	wdata = CS_BIT_MASK & (cmdArr[i] | 0xFF00);
	//	write(fd,&wdata,2);
	//	printf("[0x%02X]",cmdArr[i]);

	}
	wdata= DEFAULT_MASK;
	write(fd,&wdata,2);
	//printf("\n");
	return TRUE;
}

int writeData(int size , unsigned char* dataArr)
{
	int i ;
	unsigned short wdata;
	
	//wdata = CS_BIT_MASK;
	//write(fd,&wdata,2);

	wdata = CS_BIT_MASK & DC_BIT_MASK;
	write(fd,&wdata,2);

	//wdata = CS_BIT_MASK & DC_BIT_MASK & WD_BIT_MASK ;
	//write(fd,&wdata,2);

	wdata = CS_BIT_MASK & DC_BIT_MASK & WD_BIT_MASK & (CMD_WRITE_RAM | 0xFF00) ;
	write(fd,&wdata,2);

	wdata = CS_BIT_MASK & DC_BIT_MASK & (CMD_WRITE_RAM | 0xFF00);
	write(fd,&wdata,2);

	wdata = CS_BIT_MASK &  (CMD_WRITE_RAM | 0xFF00);
	write(fd,&wdata,2);

	for (i = 0; i < size ; i++ )
	{
		wdata = CS_BIT_MASK & WD_BIT_MASK ;
		write(fd,&wdata,2);

		wdata = CS_BIT_MASK & WD_BIT_MASK & ((unsigned char)dataArr[i] | 0xFF00 );
		write(fd,&wdata,2);

		wdata = CS_BIT_MASK & ( (unsigned char)dataArr[i] | 0xFF00);
		write(fd,&wdata,2);


	}
	wdata = DEFAULT_MASK;
	write(fd,&wdata,2);

	return TRUE;

}

int readData(int size , unsigned short* dataArr)
{

	int i ;
	unsigned short wdata;

	wdata = CS_BIT_MASK & DC_BIT_MASK;
	write(fd,&wdata,2);

	wdata = CS_BIT_MASK & DC_BIT_MASK & ( CMD_READ_RAM| 0xFF00) ;
	write(fd,&wdata,2);

	wdata = CS_BIT_MASK & DC_BIT_MASK & WD_BIT_MASK &( CMD_READ_RAM| 0xFF00);
	write(fd,&wdata,2);

	wdata = CS_BIT_MASK & DC_BIT_MASK & (CMD_READ_RAM | 0xFF00);
	write(fd,&wdata,2);

	wdata = CS_BIT_MASK &  (CMD_READ_RAM | 0xFF00);
	write(fd,&wdata,2);


	for (i = 0; i < size ; i++ )
	{
		//wdata = CS_BIT_MASK ;
		//write(fd,&wdata,2);

		wdata = CS_BIT_MASK ;
		write(fd,&wdata,2);

		wdata = CS_BIT_MASK & RD_BIT_MASK ;
		write(fd,&wdata,2);

		wdata = CS_BIT_MASK & RD_BIT_MASK ;
		write(fd,&wdata,2);

		wdata = CS_BIT_MASK ;
		write(fd,&wdata,2);

		read(fd,&dataArr[i],2);

		//wdata = CS_BIT_MASK ;
		//write(fd,&wdata,2);

	}
	wdata = DEFAULT_MASK;
	write(fd,&wdata ,2);

	return TRUE;
}

int setAddressDefalut(void)
{
	unsigned short  cmd[3];
	cmd[0] = CMD_SET_COLUMN_ADDR;
	cmd[1] = 0;
	cmd[2] = 127;
	writeCmd(3,cmd);

	cmd[0] = CMD_SET_ROW_ADDR;
	cmd[1] = 0;
	cmd[2] = 127;
	writeCmd(3,cmd);

	return TRUE;
}

// to send cmd  , must unlock
int setCmdLock(int bLock)
{
	unsigned short  cmd[3];
	
	cmd[0] = CMD_LOCK;
	if (bLock)
	{
		cmd[1] = 0x16; // lock
		writeCmd(2,cmd);

	}
	else
	{
		cmd[1] = 0x12; // lock
		writeCmd(2,cmd);

		// A2,B1,B3,BB,BE accessible
		cmd[1] = 0xB1;
		writeCmd(2,cmd);
	}
	return TRUE;
}

int imageLoading(char* fileName)
{
	int imgfile;
	unsigned char* data =NULL;
	int  width , height;

	imgfile = open(fileName , O_RDONLY );
	if ( imgfile < 0 ) 
	{
		printf ("imageloading(%s)  file is not exist . err.\n",fileName);
		return FALSE;
	}
	setCmdLock(FALSE);


	read(imgfile ,&width , sizeof(unsigned char));
	read(imgfile ,&height , sizeof(unsigned char));

	data = malloc( 128 * 128 * 3 );

	read(imgfile, data , 128 * 128 *3 );

	close(imgfile);

	writeData(128 * 128 *3 , data );

	setCmdLock(TRUE);
	return TRUE;
}

static unsigned short gamma[64]= 
{
0xB8,
0x02, 0x03, 0x04, 0x05,
0x06, 0x07, 0x08, 0x09,
0x0A, 0x0B, 0x0C, 0x0D,
0x0E, 0x0F, 0x10, 0x11,
0x12, 0x13, 0x15, 0x17,
0x19, 0x1B, 0x1D, 0x1F,
0x21, 0x23, 0x25, 0x27,
0x2A, 0x2D, 0x30, 0x33,
0x36, 0x39, 0x3C, 0x3F,
0x42, 0x45, 0x48, 0x4C,
0x50, 0x54, 0x58, 0x5C,
0x60, 0x64, 0x68, 0x6C,
0x70, 0x74, 0x78, 0x7D,
0x82, 0x87, 0x8C, 0x91,
0x96, 0x9B, 0xA0, 0xA5,
0xAA, 0xAF, 0xB4

};


int Init(void)
{
	unsigned short wdata[10];
	unsigned char  wcdata[10];
	int i,j;
	wdata[0]= 0xFD;
	wdata[1] = 0x12;
	writeCmd(2,wdata);

	
	wdata[0] = 0xFD;
	wdata[1] = 0xB1;
	writeCmd(2,wdata);

	wdata[0] = 0xAE;
	writeCmd(1,wdata);

	wdata[0] = 0xB3;
	wdata[1] = 0xF1;
	writeCmd(2,wdata);

	wdata[0] = 0xCA;
	wdata[1] = 0x7F;
	writeCmd(2,wdata);

	wdata[0] = 0xA2;
	wdata[1] = 0x00;
	writeCmd(2,wdata);

	wdata[0]= 0xA1;
	wdata[1]=0x00;
	writeCmd(2,wdata);

	wdata[0]= 0xA0;
	wdata[1] = 0xB4;
	writeCmd(2,wdata);

	wdata[0] = 0xAB;
	wdata[1] = 0x01;
	writeCmd(2,wdata);

	wdata[0] = 0xB4;
	wdata[1] = 0xA0;
	wdata[2] = 0xB5;
	wdata[3] = 0x55;
	writeCmd(4,wdata);

	wdata[0] = 0xC1;
	wdata[1] = 0xC8;
	wdata[2] = 0x80;
	wdata[3] = 0xC8;
	writeCmd(4,wdata);

	wdata[0] = 0xC7;
	wdata[1] = 0x0F;
	writeCmd(2,wdata);

	// gamma setting 
	writeCmd(64,gamma);


	wdata[0] = 0xB1;
	wdata[1] = 0x32;
	writeCmd(2,wdata);

	wdata[0] = 0xB2;
	wdata[1] = 0xA4;
	wdata[2] = 0x00;
	wdata[3] = 0x00;
	writeCmd(4,wdata);

	wdata[0] = 0xBB;
	wdata[1] = 0x17;
	writeCmd(2,wdata);

	wdata[0] = 0xB6;
	wdata[1] = 0x01;
	writeCmd(2, wdata);

	wdata[0]= 0xBE;
	wdata[1] = 0x05;
	writeCmd(2, wdata);

	wdata[0] = 0xA6;
	writeCmd(1,wdata);
	

	for (i = 0; i < 128;i++ )
	{
		for(j = 0; j < 128; j++ )
		{
			wcdata[0]= 0x3F;
			wcdata[1]= 0;
			wcdata[2] = 0;
			writeData(3,wcdata);
		}
	
	}

	wdata[0] = 0xAF;
	writeCmd(1,wdata);





	return TRUE;
}

#define MODE_WRITE		0
#define MODE_READ		1
#define MODE_CMD		2
#define MODE_RESET		3
#define MODE_IMAGE		4
#define MODE_INIT		5


static int Mode;

int main(int argc , char **argv)
{
	int writeNum;
	unsigned char wdata[10];
	int readNum;
	unsigned short* rdata = NULL;
	unsigned short wCmd[10];
	
	
	if (argc < 2 )
	{
		perror(" Args number is less than 2\n");
		doHelp();
		return 1;
	}
	
	if ( argv[1][0] == 'w')
	{
		int i ,j;
		Mode = MODE_WRITE;
		if (argc < 3)
		{
			perror(" Args number is less than 3\n");
			doHelp();
			return 1;
		}
		j = 0;
		for ( i  = 2; i < argc ; i++ )
		{
			wdata[j] = (unsigned char)read_hex(argv[i]);
			j++;
		}
		writeNum = j;
	}
	else if ( argv[1][0] == 'r')
	{
		Mode = MODE_READ;
		if ( argc < 3 ) 
		{
			perror(" Args number is less than 3\n");
			doHelp();
			return 1;
		}
		readNum = read_hex(argv[2]);

		rdata = malloc(readNum);
	}
	else if ( argv[1][0] == 'c')
	{
		int i ,j;
		Mode = MODE_CMD;
		if (argc < 3)
		{
			perror(" Args number is less than 3\n");
			doHelp();
			return 1;
		}
		j = 0;
		for ( i  = 2; i < argc ; i++ )
		{
			
			wCmd[j] = (unsigned short)read_hex(argv[i]);
			j++;
		}
		writeNum = j;
	}
	else if ( argv[1][0] == 't')
	{
		Mode = MODE_RESET;
	}
	else if ( argv[1][0] == 'd')
	{
		Mode = MODE_IMAGE;
	}
	else if (argv[1][0] == 'i')
	{
		Mode = MODE_INIT;
	}
	else
	{
		perror("No supported options.\n");
		doHelp();
		return 1;
		
	}
	
	// open  driver 
	fd = open(DRIVER_NAME,O_RDWR);
	if ( fd < 0 )
	{
		perror("driver open error.\n");
		return 1;
	}

	switch ( Mode ) 
	{
	case MODE_WRITE:
		writeData(writeNum, wdata);
		break;
	case MODE_READ:
		{
			int i;
			readData(readNum, rdata);
			printf("Read Data:\n");
			for(i =0 ; i < readNum ; i++ )
			{
				printf("[%02X]",(unsigned char)rdata[i]);
			}
			printf("\n");
		}
		break;
	case MODE_CMD:
		writeCmd(writeNum , wCmd);
		break;
	case MODE_RESET:
		reset();
		break;
	case MODE_IMAGE:
		imageLoading(argv[2]);
		break;
	case MODE_INIT:
		Init();
		break;
	}



	close(fd);
	
	if ( Mode == MODE_READ)
	{
		if ( rdata != NULL)
			free(rdata);

	}


	return 0;
}
