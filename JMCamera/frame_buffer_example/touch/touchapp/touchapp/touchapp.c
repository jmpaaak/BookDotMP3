#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // for open/close
#include <fcntl.h>      // for O_RDWR
#include <sys/ioctl.h>  // for ioctl
#include <sys/mman.h>
#include <string.h>
#include <linux/input.h>
#include <linux/fb.h>   // for fb_var_screeninfo, FBIOGET_VSCREENINFO

#define	 FBDEV_FILE "/dev/fb0"

#define  INPUT_DEVICE_LIST	"/proc/bus/input/devices"

#define  EVENT_STR	"/dev/input/event"

#define  MAX_BUFF	200

int		screen_width;
int		screen_height;
int		bits_per_pixel;
int		line_length;

#define MAX_TOUCH_X	0x740
#define MAX_TOUCH_Y	0x540

void readFirstCoordinate(int fd, int* cx , int* cy)
{
	struct input_event event;
	int readSize;
	while(1)
	{
		readSize = read(fd, &event, sizeof(event));

		if ( readSize == sizeof(event) )
		{
//			printf("type :%04X \n",event.type);
//			printf("code :%04X \n",event.code);
//			printf("value:%08X \n",event.value);
			if( event.type == EV_ABS )
			{
				if (event.code == ABS_MT_POSITION_X )
				{
					*cx = event.value*screen_width/MAX_TOUCH_X; 
				}
				else if ( event.code == ABS_MT_POSITION_Y )
				{
					*cy = event.value*screen_height/MAX_TOUCH_Y;
				}
			}
			else if ((event.type == EV_SYN) && (event.code == SYN_REPORT ))
			{
				break;
			}

		}
//		printf("\n");
	}
}

void initScreen(unsigned char *fb_mem )
{
    int		coor_y;
    int		coor_x;
    unsigned long *ptr;

    for(coor_y = 0; coor_y < screen_height; coor_y++)
    {
        ptr =   (unsigned long *)fb_mem + screen_width * coor_y ;
        for(coor_x = 0; coor_x < screen_width; coor_x++)
        {
            *ptr++  =   0x000000;
        }
    }
}


void drawRect(unsigned char *fb_mem , int sx , int sy, int ex, int ey,  unsigned long color)
{
    int x,y;
    unsigned long *ptr;

    for(y = sy; y < ey; y++)
    {
        ptr = (unsigned long*)fb_mem + screen_width * y ;

        for (x = sx; x < ex; x++)
        {
            *(ptr + x)  =   color;
        }	
    }

}
#define CUSOR_THICK		10
void drawCoordinate(unsigned char *fb_mem , int cx , int cy, int prex , int prey)
{
	int sx,sy,ex,ey;

	sx = prex - CUSOR_THICK;
	sy = prey - CUSOR_THICK;
	ex = prex + CUSOR_THICK;
	ey = prey + CUSOR_THICK;

	// erase previous cross
	if ( sx < 0 )
		sx = 0;

	if ( sy < 0 )
		sy = 0;

	if (ex >= screen_width)
		ex = screen_width - 1;

	if (ey >= screen_height)
		ey = screen_height -1;

	drawRect(fb_mem, sx, sy, ex, ey, 0x00000000);

	// draw current cross
	sx = cx - CUSOR_THICK;
	sy = cy - CUSOR_THICK;
	ex = cx + CUSOR_THICK;
	ey = cy + CUSOR_THICK;

	// erase previous cross
	if ( sx < 0 )
		sx = 0;

	if ( sy < 0 )
		sy = 0;

	if (ex >= screen_width)
		ex = screen_width - 1;

	if (ey >= screen_height)
		ey = screen_height -1;

	drawRect(fb_mem, sx, sy, ex, ey, 0xFFFFFFFF);
	printf("corX:%d, corY:%d\n",cx,cy);
}


int main(int argc, char *argv[])
{

	char	eventFullPathName[100];
	int	eventnum;
	int	x,y,prex = 0,prey = 0;
	int	fb_fd,fp;

    	struct  fb_var_screeninfo fbvar;
    	struct  fb_fix_screeninfo fbfix;
    	unsigned char   *fb_mapped;
    	int		mem_size;


	if (argc != 2)
	{
		printf("Usage : touchapp  <event Number>\n");
		printf("To see the event number, do \"cat /proc/bus/input/devices\" \n");

		return 1;
	}
	eventnum = atoi(argv[1]);	

	sprintf(eventFullPathName,"%s%d",EVENT_STR,eventnum);

	printf("touch input event name:%s\n", eventFullPathName);

	fp = open( eventFullPathName, O_RDONLY);
	if (-1 == fp)
	{
		printf("%s open fail\n",eventFullPathName);
		return 1;
	}

    if( access(FBDEV_FILE, F_OK) )
    {
        printf("%s: access error\n", FBDEV_FILE);
		close(fp);
        return 1;
    }

    if( (fb_fd = open(FBDEV_FILE, O_RDWR)) < 0)
    {
        printf("%s: open error\n", FBDEV_FILE);
		close(fp);
        return 1;
    }

    if( ioctl(fb_fd, FBIOGET_VSCREENINFO, &fbvar) )
    {
        printf("%s: ioctl error - FBIOGET_VSCREENINFO \n", FBDEV_FILE);
        goto fb_err;
    }

    if( ioctl(fb_fd, FBIOGET_FSCREENINFO, &fbfix) )
    {
        printf("%s: ioctl error - FBIOGET_FSCREENINFO \n", FBDEV_FILE);
        goto fb_err;
    }

    screen_width    =   fbvar.xres;
    screen_height   =   fbvar.yres;
    bits_per_pixel  =   fbvar.bits_per_pixel;
    line_length     =   fbfix.line_length;

    printf("screen_width : %d\n", screen_width);
    printf("screen_height : %d\n", screen_height);
    printf("bits_per_pixel : %d\n", bits_per_pixel);
    printf("line_length : %d\n", line_length);

    mem_size    =   screen_width * screen_height * 4;
    fb_mapped   =   (unsigned char *)mmap(0, mem_size,
                     PROT_READ|PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (fb_mapped < 0)
    {
        printf("mmap error!\n");
        goto fb_err;
    }

    initScreen(fb_mapped);

	while(1)
	{
		readFirstCoordinate(fp,&x, &y);

		drawCoordinate(fb_mapped,x,y, prex, prey);
		prex = x;
		prey = y;
	}

fb_err:
	close(fb_fd);
	close(fp);

	return 0;

}
