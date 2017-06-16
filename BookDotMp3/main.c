// main.c
#include <stdio.h>
#include <stdlib.h>     // for exit
#include <unistd.h>     // for open/close
#include <fcntl.h>      // for O_RDWR
#include <sys/ioctl.h>  // for ioctl
#include <sys/mman.h>
#include <linux/fb.h>   // for fb_var_screeninfo, FBIOGET_VSCREENINFO
#include <linux/input.h>
#include <pthread.h>
#include "bitmap.h"
#include "main.h"
#include "camera.h"
#include "arduino.h"
#include "speech.h"
#include "vision.h"
#include "play.h"
#include "dipswitch.h"
#include "sevenSegs.h"
#include "dotmatrix.h"
#include "colorLed.h"
#include "busLed.h"
#include "textLcd.h"
#include "oLed.h"
#include "keyMatrix.h"
#include "getAllBooks.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// screen info
int screen_width;
int screen_height;
int bits_per_pixel;
int line_length;
int mem_size;
int fbfd;
int touch_fp;
unsigned char *pfbmap;

char **books;
int curDirIndex = 0;
int maxNumBooks = 0;

// music player setting variables
int player_volume = 5;
int player_speed = 5;
int player_gender = 0; /* 0-woman, 1-man */

void callback(int curKey)
{
	char coverName[300] ="";
	char baseCommand[300] ="";

	switch(curKey) {
		case 3:
			if(curDirIndex == 0) {
				curDirIndex = maxNumBooks;
			} else {
				curDirIndex--;
			}
			strcpy(coverName ,"/cover.jpg.img");
			strcpy(baseCommand , "wget -O cover.jpg.img http://54.251.159.248/BookDotMP3/");
			strcat(baseCommand, books[curDirIndex]);
			strcat(baseCommand, coverName);
			system(baseCommand); 			// get cover.jpg.img
			loadImgOLed("./cover.jpg.img"); // set cover.jpg.img
			loadTextTLcd(books[curDirIndex]); 			// set book name 
			break;
		case 4:
			if(curDirIndex == maxNumBooks) {
				curDirIndex = 0;
			} else {
				curDirIndex++;
			}
			strcpy(coverName ,"/cover.jpg.img");
			strcpy(baseCommand ,"wget -O cover.jpg.img http://54.251.159.248/BookDotMP3/");
			strcat(baseCommand, books[curDirIndex]);
			strcat(baseCommand, coverName);
			system(baseCommand); 			// get cover.jpg.img
			loadImgOLed("./cover.jpg.img"); // set cover.jpg.img
			loadTextTLcd(books[curDirIndex]); 			// set book name 
			break;
		case 7:
			if(player_speed > 1) player_speed--;
			setBusLedNum((int)(((float)player_speed)/10.0 * 8.0));
			break;
		case 8:
			if(player_speed < 10) player_speed++;
			setBusLedNum((int)(((float)player_speed)/10.0 * 8.0));
			break;
		case 11:
			if(player_volume > 1) player_volume--;
			setDotMatrix(player_volume);
			break;
		case 12:
			if(player_volume <10) player_volume++;
			setDotMatrix(player_volume);
			break;
		case 16:
			if(player_gender==0) player_gender=1;
			else player_gender=0;
			break;
	}
}

int seven_seg_init_flag = 0;
int oled_init_flag = 0;

int main (int argc, char **argv)
{
    // touch variables
    char eventFullPathName[100]= EVENT_STR;
    int touch_x,touch_y, touch_prex=0, touch_prey=0;

    // frame variables	
    int i, j, k, t;
    int coor_x, coor_y;
    int logo_cols=0, logo_rows=0;
    int play_button_cols=0, play_button_rows=0;
    int setting_button_cols=0, setting_button_rows=0;
    
    char *logo_pData, *logo_data;
    char logo_name[200] = "./ui_images/logo.bmp";
    char *play_button_pData, *play_button_data;
    char play_button_name[200] = "./ui_images/play_button.bmp";
    char *setting_button_pData, *setting_button_data;
    char setting_button_name[200] = "./ui_images/setting_button.bmp";
    
    unsigned long logo_bmpdata[640*800];
    unsigned long logo_pixel;
    unsigned long play_button_bmpdata[200*200];
    unsigned long play_button_pixel;
    unsigned long setting_button_bmpdata[200*200];
    unsigned long setting_button_pixel;

    char r,g,b;
    // Screen Pointers
    unsigned long   *ptr;
    struct  fb_var_screeninfo fbvar;
    struct  fb_fix_screeninfo fbfix;
 

    /*
	Initialzize Text Lcd to Application Name
     */
    initTextTLcd();

    /*
       	Initialize Dotmatrix to default volume
     */
    setDotMatrix(player_volume);

    /*
	Initialize Color Led to ready start
    */
    setColorLed("wait");

    /*
        Initialize Bus Led to default speed
     */
    setBusLedNum((int)(((float)player_speed)/10.0 * 8.0));

    /*
	Initialize Get All Book List From Server
     */

    books = getAllBooks();
    curDirIndex = 0;
    maxNumBooks = sizeof(books)/4; 
    
    initKeyMatrix();

    char coverName[300] = "/cover.jpg.img";
    char baseCommand[300] = "wget -O cover.jpg.img http://54.251.159.248/BookDotMP3/";
    
    strcat(baseCommand, books[curDirIndex]);
    strcat(baseCommand, coverName);
    system(baseCommand); // get cover.jpg.img
    loadImgOLed("./cover.jpg.img"); // set cover.jpg.img
    loadTextTLcd(books[curDirIndex]); 	// set book name

    // Start Up Shell Script!
    //system("./scripts/start-up.sh");

    // Set Arduino Actuators Ready
    //arduino_serial_write("R");

    // touch fp open
    touch_fp = open(eventFullPathName, O_RDONLY);
    if(touch_fp == -1)
    {
	    printf("%s touch open fail!!\n", eventFullPathName);
	    return 1;
    }

    // touch acess
    if(access(FBDEV_FILE, F_OK))
    {
	    printf("%s touch access error!\n", FBDEV_FILE);
	    close(touch_fp);
	    return 1;
    }

    // read logo bitmap file
    read_bmp(logo_name, &logo_pData, &logo_data, &logo_cols, &logo_rows);
    
    // get logo bitmap data
    for(j = 0; j < logo_rows; j++)
    {
        k   =   j * logo_cols * 3;
        t   =   (logo_rows - 1 - j) * logo_cols;

        for(i = 0; i < logo_cols; i++)
        {
            b   =   *(logo_data + (k + i * 3));
            g   =   *(logo_data + (k + i * 3 + 1));
            r   =   *(logo_data + (k + i * 3 + 2));

            logo_pixel = ((r<<16) | (g<<8) | b);
            logo_bmpdata[t+i] = logo_pixel;          // save bitmap data bottom up
        }
    }

    close_bmp(&logo_pData);
    
    // read play button bitmap file
    read_bmp(play_button_name, &play_button_pData, \
		    &play_button_data, &play_button_cols, &play_button_rows);
    
    // get play button bitmap data
    for(j = 0; j < play_button_rows; j++)
    {
        k   =   j * play_button_cols * 3;
        t   =   (play_button_rows - 1 - j) * play_button_cols;

        for(i = 0; i < play_button_cols; i++)
        {
            b   =   *(play_button_data + (k + i * 3));
            g   =   *(play_button_data + (k + i * 3 + 1));
            r   =   *(play_button_data + (k + i * 3 + 2));

            play_button_pixel = ((r<<16) | (g<<8) | b);
            play_button_bmpdata[t+i] = play_button_pixel;          // save bitmap data bottom up
        }
    }

    close_bmp(&play_button_pData);

    // read setting button bitmap file
    read_bmp(setting_button_name, &setting_button_pData, \
		    &setting_button_data, &setting_button_cols, &setting_button_rows);
    
    // get setting button bitmap data
    for(j = 0; j < setting_button_rows; j++)
    {
        k   =   j * setting_button_cols * 3;
        t   =   (setting_button_rows - 1 - j) * setting_button_cols;

        for(i = 0; i < setting_button_cols; i++)
        {
            b   =   *(setting_button_data + (k + i * 3));
            g   =   *(setting_button_data + (k + i * 3 + 1));
            r   =   *(setting_button_data + (k + i * 3 + 2));

            setting_button_pixel = ((r<<16) | (g<<8) | b);
            setting_button_bmpdata[t+i] = setting_button_pixel;          // save bitmap data bottom up
        }
    }

    close_bmp(&setting_button_pData);

    // file open stuff
    if( (fbfd = open(FBDEV_FILE, O_RDWR)) < 0)
    {
        printf("%s: open error\n", FBDEV_FILE);
        exit(1);
    }

    if( ioctl(fbfd, FBIOGET_VSCREENINFO, &fbvar) )
    {
        printf("%s: ioctl error - FBIOGET_VSCREENINFO \n", FBDEV_FILE);
        exit(1);
    }

    if( ioctl(fbfd, FBIOGET_FSCREENINFO, &fbfix) )
    {
        printf("%s: ioctl error - FBIOGET_FSCREENINFO \n", FBDEV_FILE);
        exit(1);
    }

    if (fbvar.bits_per_pixel != 32)
    {
        fprintf(stderr, "bpp is not 32\n");
        exit(1);
    }

    // screen information
    screen_width    =   fbvar.xres;
    screen_height   =   fbvar.yres;
    bits_per_pixel  =   fbvar.bits_per_pixel;
    line_length     =   fbfix.line_length;
    mem_size    =   line_length * screen_height;

    // screen map base pointer
    pfbmap  =   (unsigned char *)
        mmap(0, mem_size, PROT_READ|PROT_WRITE, MAP_SHARED, fbfd, 0);

    if ((unsigned)pfbmap == (unsigned)-1)
    {
        perror("fbdev mmap\n");
        exit(1);
    }

    // fb background clear 
    for(coor_y = 0; coor_y < screen_height; coor_y++) {
        ptr =   (unsigned long *)pfbmap + (screen_width * coor_y);
       
	// logo below background
	for(coor_x = 0; coor_x < screen_width-logo_rows-2; coor_x++)
        {
            *ptr++  =   0xffffff;
        }

	// logo part background
	for(coor_x = screen_width-logo_rows-2; coor_x < screen_width; coor_x++)
        {
            *ptr++  =   0xaaaaaa;
        }
    }

    // display logo
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < logo_cols; coor_y++) {
	if(logo_cols%2 != 0) {
       	   ptr = (unsigned long*)pfbmap + screen_width*(coor_y+screen_height/2-logo_cols/2-1) \
		   + screen_width-logo_rows;
	}
	else {
           ptr = (unsigned long*)pfbmap + screen_width*(coor_y+screen_height/2-logo_cols/2) \
		   + screen_width-logo_rows;
        }
	for (coor_x = logo_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = logo_bmpdata[coor_y + coor_x*logo_cols];
        }
    }

    // play intro mp3 file
    playMp3File("./ui_mp3s/intro.mp3");

    // display setting button
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < setting_button_cols; coor_y++) {
	if(setting_button_cols%2 != 0) {
       	   ptr = (unsigned long*)pfbmap + screen_width * \
		 (coor_y+screen_height/2-setting_button_cols/2-1) + \
		 (screen_width-logo_rows)/5*4 - setting_button_rows;
	}
	else {
           ptr = (unsigned long*)pfbmap + screen_width * \
	   	 (coor_y+screen_height/2-setting_button_cols/2) + \
		 (screen_width-logo_rows)/5*4 - setting_button_rows;
        }
	for (coor_x = setting_button_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = setting_button_bmpdata[coor_y + coor_x*setting_button_cols];
        }
    }

    // display play button
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < play_button_cols; coor_y++) {
	if(play_button_cols%2 != 0) {
       	   ptr = (unsigned long*)pfbmap + screen_width * \
	   	 (coor_y+screen_height/2-play_button_cols/2-1) + \
		 (screen_width-logo_rows)/5*2 - play_button_rows;
	}
	else {
           ptr = (unsigned long*)pfbmap + screen_width * \
	   	 (coor_y+screen_height/2-play_button_cols/2) + \
		 (screen_width-logo_rows)/5*2 - play_button_rows;
        }
	for (coor_x = play_button_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = play_button_bmpdata[coor_y + coor_x*play_button_cols];
        }
    }

    int setting_button_center_y = screen_height/2;
    int setting_button_start_x = (screen_width-logo_rows)/5*4 - \
				 setting_button_rows;
    int play_button_center_y = screen_height/2;
    int play_button_start_x = (screen_width-logo_rows)/5*2 - \
			      play_button_rows;

    // touch event wating loop!
    while(1)
    {
	    // read touch event & coordinates
	    readFirstCoordinate(touch_fp, &touch_x, &touch_y);
	    touch_prex = touch_x;
	    touch_prey = touch_y;
	    char main_refresh_flag=0;
	    
	    // see which part is touched
	    // setting button part
	    if((touch_y >= setting_button_center_y-setting_button_cols/2) && \
	       (touch_y <= setting_button_center_y+setting_button_cols/2) && \
	       (touch_x >= setting_button_start_x) && \
	       (touch_x <= setting_button_start_x+setting_button_rows))
	    {
		   showSettingMenu(&player_volume, &player_speed, &player_gender);
		   main_refresh_flag=1;
	    }

	    // play button part
	    if((touch_y >= play_button_center_y-play_button_cols/2) && \
	       (touch_y <= play_button_center_y+play_button_cols/2) && \
	       (touch_x >= play_button_start_x) && \
	       (touch_x <= play_button_start_x+play_button_rows))
	    {
		   showPlayMenu(player_volume, player_speed, player_gender);
		   main_refresh_flag=1;
	    }

	   if(main_refresh_flag==1) {	          
              // fb background clear 
    	      for(coor_y = 0; coor_y < screen_height; coor_y++) {
                 ptr =   (unsigned long *)pfbmap + (screen_width * coor_y);
       
    	         // logo below background
	         for(coor_x = 0; coor_x < screen_width-logo_rows-2; coor_x++)
                 {
                    *ptr++  =   0xffffff;
                 }

	         // logo part background
	         for(coor_x = screen_width-logo_rows-2; coor_x < screen_width; coor_x++)
                 {
                    *ptr++  =   0xaaaaaa;
                 }
              }

              // display logo
              // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
              for(coor_y = 0; coor_y < logo_cols; coor_y++) {
	         if(logo_cols%2 != 0) {
       	            ptr = (unsigned long*)pfbmap + screen_width*(coor_y+screen_height/2-\
		   		                logo_cols/2-1) + screen_width-logo_rows;
	         }
	         else {
                    ptr = (unsigned long*)pfbmap + screen_width*(coor_y+screen_height/2-\
				       logo_cols/2) + screen_width-logo_rows;
                 }
	         for (coor_x = logo_rows-1; coor_x >= 0; coor_x--) {
                     *ptr++  = logo_bmpdata[coor_y + coor_x*logo_cols];
                 }
              }

              // display setting button
              // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
              for(coor_y = 0; coor_y < setting_button_cols; coor_y++) {
	          if(setting_button_cols%2 != 0) {
       	             ptr = (unsigned long*)pfbmap + screen_width * \
		           (coor_y+screen_height/2-setting_button_cols/2-1) + \
		           (screen_width-logo_rows)/5*4 - setting_button_rows;
	          }
	          else {
                     ptr = (unsigned long*)pfbmap + screen_width * \
	   	           (coor_y+screen_height/2-setting_button_cols/2) + \
		           (screen_width-logo_rows)/5*4 - setting_button_rows;
                  }
	          for (coor_x = setting_button_rows-1; coor_x >= 0; coor_x--) {
                      *ptr++  = setting_button_bmpdata[coor_y + coor_x*setting_button_cols];
                  }
              }

              // display play button
              // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
              for(coor_y = 0; coor_y < play_button_cols; coor_y++) {
	          if(play_button_cols%2 != 0) {
       	             ptr = (unsigned long*)pfbmap + screen_width * \
	   	           (coor_y+screen_height/2-play_button_cols/2-1) + \
		           (screen_width-logo_rows)/5*2 - play_button_rows;
	          }
	          else {
                     ptr = (unsigned long*)pfbmap + screen_width * \
	   	           (coor_y+screen_height/2-play_button_cols/2) + \
		           (screen_width-logo_rows)/5*2 - play_button_rows;
                  }
	          for (coor_x = play_button_rows-1; coor_x >= 0; coor_x--) {
                      *ptr++  = play_button_bmpdata[coor_y + coor_x*play_button_cols];
                  }
	      }
	   }
    }

    munmap(pfbmap, mem_size);
    close(fbfd);
    close(touch_fp);
    return 0;
}



void read_bmp(char *filename, char **pDib, char **data, int *cols, int *rows)
{
    BITMAPFILEHEADER    bmpHeader;
    BITMAPINFOHEADER    *bmpInfoHeader;
    unsigned int    size;
    unsigned char   magicNum[2];
    int     nread;
    FILE    *fp;

    fp  =  fopen(filename, "rb");
    if(fp == NULL) {
        printf("ERROR\n");
        return;
    }

    // identify bmp file
    magicNum[0]   =   fgetc(fp);
    magicNum[1]   =   fgetc(fp);

    if(magicNum[0] != 'B' && magicNum[1] != 'M') {
        printf("It's not a bmp file!\n");
        fclose(fp);
        return;
    }

    nread   =   fread(&bmpHeader.bfSize, 1, sizeof(BITMAPFILEHEADER), fp);
    size    =   bmpHeader.bfSize - sizeof(BITMAPFILEHEADER);
    *pDib   =   (unsigned char *)malloc(size);      // DIB Header(Image Header)
    fread(*pDib, 1, size, fp);
    bmpInfoHeader   =   (BITMAPINFOHEADER *)*pDib;

    // check 24bit
    if(BIT_VALUE_24BIT != (bmpInfoHeader->biBitCount))     // bit value
    {
        printf("It supports only 24bit bmp!\n");
        fclose(fp);
        return;
    }

    *cols   =   bmpInfoHeader->biWidth;
    *rows   =   bmpInfoHeader->biHeight;
    *data   =   (char *)(*pDib + bmpHeader.bfOffBits - sizeof(bmpHeader) - 2);
    fclose(fp);
}

void close_bmp(char **pDib)     // DIB(Device Independent Bitmap)
{
    free(*pDib);
}

void readFirstCoordinate(int fd, int* cx, int* cy) 
{
    struct input_event event;
    int readSize;
    while(1)
    {
	    readSize = read(fd, &event, sizeof(event));
	    if(readSize == sizeof(event))
	    {
		    if(event.type == EV_ABS)
		    {
			    if(event.code == ABS_MT_POSITION_X)
			    {
			    	*cx = event.value*screen_width/MAX_TOUCH_X;
			    }
			    else if(event.code == ABS_MT_POSITION_Y)
			    {
				*cy = event.value*screen_height/MAX_TOUCH_Y;
			    }
	            } 
	   	    else if((event.type == EV_SYN)&&(event.code == SYN_REPORT))
		    {
			    break;
		    }
	    }
    }
}

void showSettingMenu(int *player_volume, int *player_speed, int *player_gender)
{
    // touch variables
    char eventFullPathName[100]= EVENT_STR;
    int touch_x,touch_y, touch_prex=0, touch_prey=0;

    // frame variables	
    int i, j, k, t;
    int coor_x, coor_y;
    int logo_cols=0, logo_rows=0;
    int play_button_cols=0, play_button_rows=0;
    int setting_button_cols=0, setting_button_rows=0;
    int volume_button_cols=0, volume_button_rows=0;
    int speed_button_cols=0, speed_button_rows=0;
    int gender_button_cols=0, gender_button_rows=0;
    int plus_button_cols=0, plus_button_rows;
    int minus_button_cols=0, minus_button_rows;
    int change_button_cols=0, change_button_rows;
    int one_cols=0, one_rows=0;
    int two_cols=0, two_rows=0;
    int three_cols=0, three_rows=0;
    int four_cols=0, four_rows=0;
    int five_cols=0, five_rows=0;
    int six_cols=0, six_rows=0;
    int seven_cols=0, seven_rows=0;
    int eight_cols=0, eight_rows=0;
    int nine_cols=0, nine_rows=0;
    int ten_cols=0, ten_rows=0;
    int man_button_cols=0, man_button_rows=0;
    int woman_button_cols=0, woman_button_rows=0;
    int fix_button_cols=0, fix_button_rows=0;
    int unfix_button_cols=0, unfix_button_rows=0;

    char *logo_pData, *logo_data;
    char logo_name[200] = "./ui_images/logo_setting.bmp";
    char *play_button_pData, *play_button_data;
    char play_button_name[200] = "./ui_images/play_button.bmp";
    char *setting_button_pData, *setting_button_data;
    char setting_button_name[200] = "./ui_images/setting_button.bmp";
    char *volume_button_pData, *volume_button_data;
    char volume_button_name[200] = "./ui_images/volume_button.bmp";
    char *speed_button_pData, *speed_button_data;
    char speed_button_name[200] = "./ui_images/speed_button.bmp";
    char *gender_button_pData, *gender_button_data;
    char gender_button_name[200] = "./ui_images/gender_button.bmp";
    char *plus_button_pData, *plus_button_data;
    char plus_button_name[200] = "./ui_images/plus_button.bmp";
    char *minus_button_pData, *minus_button_data;
    char minus_button_name[200] = "./ui_images/minus_button.bmp";
    char *change_button_pData, *change_button_data;
    char change_button_name[200] = "./ui_images/change_button.bmp";
    char *one_pData, *one_data;
    char one_name[200] = "./ui_images/one.bmp";
    char *two_pData, *two_data;
    char two_name[200] = "./ui_images/two.bmp";
    char *three_pData, *three_data;
    char three_name[200] = "./ui_images/three.bmp";
    char *four_pData, *four_data;
    char four_name[200] = "./ui_images/four.bmp";
    char *five_pData, *five_data;
    char five_name[200] = "./ui_images/five.bmp";
    char *six_pData, *six_data;
    char six_name[200] = "./ui_images/six.bmp";
    char *seven_pData, *seven_data;
    char seven_name[200] = "./ui_images/seven.bmp";
    char *eight_pData, *eight_data;
    char eight_name[200] = "./ui_images/eight.bmp";
    char *nine_pData, *nine_data;
    char nine_name[200] = "./ui_images/nine.bmp";
    char *ten_pData, *ten_data;
    char ten_name[200] = "./ui_images/ten.bmp";
    char *man_button_pData, *man_button_data;
    char man_button_name[200] = "./ui_images/man_button.bmp";
    char *woman_button_pData, *woman_button_data;
    char woman_button_name[200] = "./ui_images/woman_button.bmp";
    char *fix_button_pData, *fix_button_data;
    char fix_button_name[200] = "./ui_images/fix_button.bmp";
    char *unfix_button_pData, *unfix_button_data;
    char unfix_button_name[200] = "./ui_images/unfix_button.bmp";

    unsigned long logo_bmpdata[640*800];
    unsigned long logo_pixel;
    unsigned long play_button_bmpdata[200*200];
    unsigned long play_button_pixel;
    unsigned long setting_button_bmpdata[200*200];
    unsigned long setting_button_pixel;
    unsigned long volume_button_bmpdata[200*200];
    unsigned long volume_button_pixel;
    unsigned long speed_button_bmpdata[200*200];
    unsigned long speed_button_pixel;
    unsigned long gender_button_bmpdata[200*200];
    unsigned long gender_button_pixel;
    unsigned long plus_button_bmpdata[200*200];
    unsigned long plus_button_pixel;
    unsigned long minus_button_bmpdata[200*200];
    unsigned long minus_button_pixel;
    unsigned long change_button_bmpdata[200*200];
    unsigned long change_button_pixel;
    unsigned long one_bmpdata[200*200];
    unsigned long one_pixel;
    unsigned long two_bmpdata[200*200];
    unsigned long two_pixel;
    unsigned long three_bmpdata[200*200];
    unsigned long three_pixel;
    unsigned long four_bmpdata[200*200];
    unsigned long four_pixel;
    unsigned long five_bmpdata[200*200];
    unsigned long five_pixel;
    unsigned long six_bmpdata[200*200];
    unsigned long six_pixel;
    unsigned long seven_bmpdata[200*200];
    unsigned long seven_pixel;
    unsigned long eight_bmpdata[200*200];
    unsigned long eight_pixel;
    unsigned long nine_bmpdata[200*200];
    unsigned long nine_pixel;
    unsigned long ten_bmpdata[200*200];
    unsigned long ten_pixel;
    unsigned long man_button_bmpdata[200*200];
    unsigned long man_button_pixel;
    unsigned long woman_button_bmpdata[200*200];
    unsigned long woman_button_pixel;
    unsigned long fix_button_bmpdata[200*400];
    unsigned long fix_button_pixel;
    unsigned long unfix_button_bmpdata[200*400];
    unsigned long unfix_button_pixel;

    unsigned long *ptr;
    char r,g,b;

    // read logo bitmap file
    read_bmp(logo_name, &logo_pData, &logo_data, &logo_cols, &logo_rows);
    
    // get logo bitmap data
    for(j = 0; j < logo_rows; j++)
    {
        k   =   j * logo_cols * 3;
        t   =   (logo_rows - 1 - j) * logo_cols;

        for(i = 0; i < logo_cols; i++)
        {
            b   =   *(logo_data + (k + i * 3));
            g   =   *(logo_data + (k + i * 3 + 1));
            r   =   *(logo_data + (k + i * 3 + 2));

            logo_pixel = ((r<<16) | (g<<8) | b);
            logo_bmpdata[t+i] = logo_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&logo_pData);

    // read volume button bitmap file
    read_bmp(volume_button_name, &volume_button_pData, \
		    &volume_button_data, &volume_button_cols, &volume_button_rows);
    
    // get volume button bitmap data
    for(j = 0; j < volume_button_rows; j++)
    {
        k   =   j * volume_button_cols * 3;
        t   =   (volume_button_rows - 1 - j) * volume_button_cols;

        for(i = 0; i < volume_button_cols; i++)
        {
            b   =   *(volume_button_data + (k + i * 3));
            g   =   *(volume_button_data + (k + i * 3 + 1));
            r   =   *(volume_button_data + (k + i * 3 + 2));

            volume_button_pixel = ((r<<16) | (g<<8) | b);
            volume_button_bmpdata[t+i] = volume_button_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&volume_button_pData);

    // read speed button bitmap file
    read_bmp(speed_button_name, &speed_button_pData, \
	    &speed_button_data, &speed_button_cols, &speed_button_rows);
    
    // get speed button bitmap data
    for(j = 0; j < speed_button_rows; j++)
    {
        k   =   j * speed_button_cols * 3;
        t   =   (speed_button_rows - 1 - j) * speed_button_cols;

        for(i = 0; i < speed_button_cols; i++)
        {
            b   =   *(speed_button_data + (k + i * 3));
            g   =   *(speed_button_data + (k + i * 3 + 1));
            r   =   *(speed_button_data + (k + i * 3 + 2));

            speed_button_pixel = ((r<<16) | (g<<8) | b);
            speed_button_bmpdata[t+i] = speed_button_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&speed_button_pData);

    // read gender button bitmap file
    read_bmp(gender_button_name, &gender_button_pData, \
		    &gender_button_data, &gender_button_cols, &gender_button_rows);
    
    // get gender button bitmap data
    for(j = 0; j < gender_button_rows; j++)
    {
        k   =   j * gender_button_cols * 3;
        t   =   (gender_button_rows - 1 - j) * gender_button_cols;

        for(i = 0; i < gender_button_cols; i++)
        {
            b   =   *(gender_button_data + (k + i * 3));
            g   =   *(gender_button_data + (k + i * 3 + 1));
            r   =   *(gender_button_data + (k + i * 3 + 2));

            gender_button_pixel = ((r<<16) | (g<<8) | b);
            gender_button_bmpdata[t+i] = gender_button_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&gender_button_pData);

    // read plus button file
    read_bmp(plus_button_name, &plus_button_pData, \
		    &plus_button_data, &plus_button_cols, &plus_button_rows);
    
    // get plus button data
    for(j = 0; j < plus_button_rows; j++)
    {
        k   =   j * plus_button_cols * 3;
        t   =   (plus_button_rows - 1 - j) * plus_button_cols;

        for(i = 0; i < plus_button_cols; i++)
        {
            b   =   *(plus_button_data + (k + i * 3));
            g   =   *(plus_button_data + (k + i * 3 + 1));
            r   =   *(plus_button_data + (k + i * 3 + 2));

            plus_button_pixel = ((r<<16) | (g<<8) | b);
            plus_button_bmpdata[t+i] = plus_button_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&plus_button_pData);

    // read minus button bitmap file
    read_bmp(minus_button_name, &minus_button_pData, \
		    &minus_button_data, &minus_button_cols, &minus_button_rows);
    
    // get minus button bitmap data
    for(j = 0; j < minus_button_rows; j++)
    {
        k   =   j * minus_button_cols * 3;
        t   =   (minus_button_rows - 1 - j) * minus_button_cols;

        for(i = 0; i < minus_button_cols; i++)
        {
            b   =   *(minus_button_data + (k + i * 3));
            g   =   *(minus_button_data + (k + i * 3 + 1));
            r   =   *(minus_button_data + (k + i * 3 + 2));

            minus_button_pixel = ((r<<16) | (g<<8) | b);
            minus_button_bmpdata[t+i] = minus_button_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&minus_button_pData);

    // read change button bitmap file
    read_bmp(change_button_name, &change_button_pData, \
		    &change_button_data, &change_button_cols, &change_button_rows);
    
    // get change button bitmap data
    for(j = 0; j < change_button_rows; j++)
    {
        k   =   j * change_button_cols * 3;
        t   =   (change_button_rows - 1 - j) * change_button_cols;

        for(i = 0; i < change_button_cols; i++)
        {
            b   =   *(change_button_data + (k + i * 3));
            g   =   *(change_button_data + (k + i * 3 + 1));
            r   =   *(change_button_data + (k + i * 3 + 2));

            change_button_pixel = ((r<<16) | (g<<8) | b);
            change_button_bmpdata[t+i] = change_button_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&change_button_pData);

    // read man button bitmap file
    read_bmp(man_button_name, &man_button_pData, \
		    &man_button_data, &man_button_cols, &man_button_rows);
    
    // get man button bitmap data
    for(j = 0; j < man_button_rows; j++)
    {
        k   =   j * man_button_cols * 3;
        t   =   (man_button_rows - 1 - j) * man_button_cols;

        for(i = 0; i < man_button_cols; i++)
        {
            b   =   *(man_button_data + (k + i * 3));
            g   =   *(man_button_data + (k + i * 3 + 1));
            r   =   *(man_button_data + (k + i * 3 + 2));

            man_button_pixel = ((r<<16) | (g<<8) | b);
            man_button_bmpdata[t+i] = man_button_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&man_button_pData);

    // read woman bitmap file
    read_bmp(woman_button_name, &woman_button_pData, \
		    &woman_button_data, &woman_button_cols, &woman_button_rows);
    
    // get woman bitmap data
    for(j = 0; j < woman_button_rows; j++)
    {
        k   =   j * woman_button_cols * 3;
        t   =   (woman_button_rows - 1 - j) * woman_button_cols;

        for(i = 0; i < woman_button_cols; i++)
        {
            b   =   *(woman_button_data + (k + i * 3));
            g   =   *(woman_button_data + (k + i * 3 + 1));
            r   =   *(woman_button_data + (k + i * 3 + 2));

            woman_button_pixel = ((r<<16) | (g<<8) | b);
            woman_button_bmpdata[t+i] = woman_button_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&woman_button_pData);

    // read one bitmap file
    read_bmp(one_name, &one_pData, &one_data, &one_cols, &one_rows);
    
    // get one bitmap data
    for(j = 0; j < one_rows; j++)
    {
        k   =   j * one_cols * 3;
        t   =   (one_rows - 1 - j) * one_cols;

        for(i = 0; i < one_cols; i++)
        {
            b   =   *(one_data + (k + i * 3));
            g   =   *(one_data + (k + i * 3 + 1));
            r   =   *(one_data + (k + i * 3 + 2));

            one_pixel = ((r<<16) | (g<<8) | b);
            one_bmpdata[t+i] = one_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&one_pData);

    // read two bitmap file
    read_bmp(two_name, &two_pData, &two_data, &two_cols, &two_rows);
    
    // get two bitmap data
    for(j = 0; j < two_rows; j++)
    {
        k   =   j * two_cols * 3;
        t   =   (two_rows - 1 - j) * two_cols;

        for(i = 0; i < two_cols; i++)
        {
            b   =   *(two_data + (k + i * 3));
            g   =   *(two_data + (k + i * 3 + 1));
            r   =   *(two_data + (k + i * 3 + 2));

            two_pixel = ((r<<16) | (g<<8) | b);
            two_bmpdata[t+i] = two_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&two_pData);

    // read three bitmap file
    read_bmp(three_name, &three_pData, &three_data, \
		    &three_cols, &three_rows);
    
    // get three bitmap data
    for(j = 0; j < three_rows; j++)
    {
        k   =   j * three_cols * 3;
        t   =   (three_rows - 1 - j) * three_cols;

        for(i = 0; i < three_cols; i++)
        {
            b   =   *(three_data + (k + i * 3));
            g   =   *(three_data + (k + i * 3 + 1));
            r   =   *(three_data + (k + i * 3 + 2));

            three_pixel = ((r<<16) | (g<<8) | b);
            three_bmpdata[t+i] = three_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&three_pData);

    // read four bitmap file
    read_bmp(four_name, &four_pData, &four_data, \
		    &four_cols, &four_rows);
    
    // get four bitmap data
    for(j = 0; j < four_rows; j++)
    {
        k   =   j * four_cols * 3;
        t   =   (four_rows - 1 - j) * four_cols;

        for(i = 0; i < four_cols; i++)
        {
            b   =   *(four_data + (k + i * 3));
            g   =   *(four_data + (k + i * 3 + 1));
            r   =   *(four_data + (k + i * 3 + 2));

            four_pixel = ((r<<16) | (g<<8) | b);
            four_bmpdata[t+i] = four_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&four_pData);

    // read five bitmap file
    read_bmp(five_name, &five_pData, &five_data, \
		    &five_cols, &five_rows);
    
    // get five bitmap data
    for(j = 0; j < five_rows; j++)
    {
        k   =   j * five_cols * 3;
        t   =   (five_rows - 1 - j) * five_cols;

        for(i = 0; i < five_cols; i++)
        {
            b   =   *(five_data + (k + i * 3));
            g   =   *(five_data + (k + i * 3 + 1));
            r   =   *(five_data + (k + i * 3 + 2));

            five_pixel = ((r<<16) | (g<<8) | b);
            five_bmpdata[t+i] = five_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&five_pData);

    // read six bitmap file
    read_bmp(six_name, &six_pData, &six_data, \
		    &six_cols, &six_rows);
    
    // get six bitmap data
    for(j = 0; j < six_rows; j++)
    {
        k   =   j * six_cols * 3;
        t   =   (six_rows - 1 - j) * six_cols;

        for(i = 0; i < six_cols; i++)
        {
            b   =   *(six_data + (k + i * 3));
            g   =   *(six_data + (k + i * 3 + 1));
            r   =   *(six_data + (k + i * 3 + 2));

            six_pixel = ((r<<16) | (g<<8) | b);
            six_bmpdata[t+i] = six_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&six_pData);

    // read seven bitmap file
    read_bmp(seven_name, &seven_pData, &seven_data, \
		    &seven_cols, &seven_rows);
    
    // get seven bitmap data
    for(j = 0; j < seven_rows; j++)
    {
        k   =   j * seven_cols * 3;
        t   =   (seven_rows - 1 - j) * seven_cols;

        for(i = 0; i < seven_cols; i++)
        {
            b   =   *(seven_data + (k + i * 3));
            g   =   *(seven_data + (k + i * 3 + 1));
            r   =   *(seven_data + (k + i * 3 + 2));

            seven_pixel = ((r<<16) | (g<<8) | b);
            seven_bmpdata[t+i] = seven_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&seven_pData);

    // read eight bitmap file
    read_bmp(eight_name, &eight_pData, &eight_data, \
		    &eight_cols, &eight_rows);
    
    // get eight bitmap data
    for(j = 0; j < eight_rows; j++)
    {
        k   =   j * eight_cols * 3;
        t   =   (eight_rows - 1 - j) * eight_cols;

        for(i = 0; i < eight_cols; i++)
        {
            b   =   *(eight_data + (k + i * 3));
            g   =   *(eight_data + (k + i * 3 + 1));
            r   =   *(eight_data + (k + i * 3 + 2));

            eight_pixel = ((r<<16) | (g<<8) | b);
            eight_bmpdata[t+i] = eight_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&eight_pData);

    // read nine bitmap file
    read_bmp(nine_name, &nine_pData, &nine_data, \
		    &nine_cols, &nine_rows);
    
    // get nine bitmap data
    for(j = 0; j < nine_rows; j++)
    {
        k   =   j * nine_cols * 3;
        t   =   (nine_rows - 1 - j) * nine_cols;

        for(i = 0; i < nine_cols; i++)
        {
            b   =   *(nine_data + (k + i * 3));
            g   =   *(nine_data + (k + i * 3 + 1));
            r   =   *(nine_data + (k + i * 3 + 2));

            nine_pixel = ((r<<16) | (g<<8) | b);
            nine_bmpdata[t+i] = nine_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&nine_pData);

    // read ten bitmap file
    read_bmp(ten_name, &ten_pData, &ten_data, &ten_cols, &ten_rows);
    
    // get ten bitmap data
    for(j = 0; j < ten_rows; j++)
    {
        k   =   j * ten_cols * 3;
        t   =   (ten_rows - 1 - j) * ten_cols;

        for(i = 0; i < ten_cols; i++)
        {
            b   =   *(ten_data + (k + i * 3));
            g   =   *(ten_data + (k + i * 3 + 1));
            r   =   *(ten_data + (k + i * 3 + 2));

            ten_pixel = ((r<<16) | (g<<8) | b);
            ten_bmpdata[t+i] = ten_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&ten_pData);

    // read fix button bitmap file
    read_bmp(fix_button_name, &fix_button_pData, \
		    &fix_button_data, &fix_button_cols, &fix_button_rows);
    
    // get fix button bitmap data
    for(j = 0; j < fix_button_rows; j++)
    {
        k   =   j * fix_button_cols * 3;
        t   =   (fix_button_rows - 1 - j) * fix_button_cols;

        for(i = 0; i < fix_button_cols; i++)
        {
            b   =   *(fix_button_data + (k + i * 3));
            g   =   *(fix_button_data + (k + i * 3 + 1));
            r   =   *(fix_button_data + (k + i * 3 + 2));

            fix_button_pixel = ((r<<16) | (g<<8) | b);
            fix_button_bmpdata[t+i] = fix_button_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&fix_button_pData);

    // read unfix button bitmap file
    read_bmp(unfix_button_name, &unfix_button_pData, \
		    &unfix_button_data, &unfix_button_cols, &unfix_button_rows);
    
    // get unfix button bitmap data
    for(j = 0; j < unfix_button_rows; j++)
    {
        k   =   j * unfix_button_cols * 3;
        t   =   (unfix_button_rows - 1 - j) * unfix_button_cols;

        for(i = 0; i < unfix_button_cols; i++)
        {
            b   =   *(unfix_button_data + (k + i * 3));
            g   =   *(unfix_button_data + (k + i * 3 + 1));
            r   =   *(unfix_button_data + (k + i * 3 + 2));

            unfix_button_pixel = ((r<<16) | (g<<8) | b);
            unfix_button_bmpdata[t+i] = unfix_button_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&unfix_button_pData);

    // fb background clear 
    for(coor_y = 0; coor_y < screen_height; coor_y++) {
        ptr =   (unsigned long *)pfbmap + (screen_width * coor_y);
       
	// logo below background
	for(coor_x = 0; coor_x < screen_width-logo_rows-2; coor_x++)
        {
            *ptr++  =   0xffffff;
        }

	// logo part background
	for(coor_x = screen_width-logo_rows-2; coor_x < screen_width; coor_x++)
        {
            *ptr++  =   0xaaaaaa;
        }
    }

    // display logo
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < logo_cols; coor_y++) {
	if(logo_cols%2 != 0) {
       	   ptr = (unsigned long*)pfbmap + screen_width*(coor_y+screen_height/2-logo_cols/2-1) \
		   + screen_width-logo_rows;
	}
	else {
           ptr = (unsigned long*)pfbmap + screen_width*(coor_y+screen_height/2-logo_cols/2) \
		   + screen_width-logo_rows;
        }
	for (coor_x = logo_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = logo_bmpdata[coor_y + coor_x*logo_cols];
        }
    }

    
    // display volume button
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < volume_button_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+20) + \
	      (screen_width-logo_rows-fix_button_rows-25)/3*4 - volume_button_rows;
       
	for (coor_x = volume_button_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = volume_button_bmpdata[coor_y + coor_x*volume_button_cols];
        }
    }

    // display speed button
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < speed_button_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+20) + \
	      (screen_width-logo_rows-fix_button_rows-25)/3*3 - speed_button_rows;
        
	for (coor_x = speed_button_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = speed_button_bmpdata[coor_y + coor_x*speed_button_cols];
        }
    }

    
    // display gender button
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < gender_button_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+20) + \
	      (screen_width-logo_rows-fix_button_rows-25)/3*2 - gender_button_rows;
        
	for (coor_x = gender_button_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = gender_button_bmpdata[coor_y + coor_x*gender_button_cols];
        }
    }
    
    // display volume_plus button
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < plus_button_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height/2 - plus_button_cols-10) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*4 -25 - plus_button_rows;
        
	for (coor_x = plus_button_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = plus_button_bmpdata[coor_y + coor_x*plus_button_cols];
        }
    }

    // display volume_minus button
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < minus_button_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height/2 + 10) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*4 -25 - minus_button_rows;
        
	for (coor_x = minus_button_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = minus_button_bmpdata[coor_y + coor_x*minus_button_cols];
        }
    }

    // display speed_plus button
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < plus_button_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height/2 -plus_button_cols-10) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*3 -25 -plus_button_rows;
        
	for (coor_x = plus_button_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = plus_button_bmpdata[coor_y + coor_x*plus_button_cols];
        }
    }

    // display speed_minus button
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < minus_button_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height/2 +10) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*3 -25 -minus_button_rows;
        
	for (coor_x = minus_button_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = minus_button_bmpdata[coor_y + coor_x*minus_button_cols];
        }
    }

    // display change button
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < change_button_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height/2 - change_button_cols/2) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*2 -25 -change_button_rows;
        
	for (coor_x = change_button_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = change_button_bmpdata[coor_y + coor_x*change_button_cols];
        }
    }

    // display fix button
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < fix_button_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height/2 - fix_button_cols/2);
        
	for (coor_x = fix_button_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = fix_button_bmpdata[coor_y + coor_x*fix_button_cols];
        }
    }

    //display volume_digit
    switch(*player_volume)
    {
    case 1:	
    for(coor_y = 0; coor_y < one_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - one_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*4 -one_rows;
        
	for (coor_x = one_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = one_bmpdata[coor_y + coor_x*one_cols];
        }
    }
    break;
    case 2:
    for(coor_y = 0; coor_y < two_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - two_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*4 -two_rows;
        
	for (coor_x = two_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = two_bmpdata[coor_y + coor_x*two_cols];
        }
    }
    break;
    case 3:
    for(coor_y = 0; coor_y < three_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - three_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*4 -three_rows;
        
	for (coor_x = three_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = three_bmpdata[coor_y + coor_x*three_cols];
        }
    }
    break;
    case 4:
    for(coor_y = 0; coor_y < four_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - four_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*4 - four_rows;
        
	for (coor_x = four_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = four_bmpdata[coor_y + coor_x*four_cols];
        }
    }
    break;
    case 5:
    for(coor_y = 0; coor_y < five_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - five_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*4 - five_rows;
        
	for (coor_x = five_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = five_bmpdata[coor_y + coor_x*five_cols];
        }
    }
    break;
    case 6:
    for(coor_y = 0; coor_y < six_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - six_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*4 - six_rows;
        
	for (coor_x = six_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = six_bmpdata[coor_y + coor_x*six_cols];
        }
    }
    break;
    case 7:
    for(coor_y = 0; coor_y < seven_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - seven_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*4 - seven_rows;
        
	for (coor_x = seven_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = seven_bmpdata[coor_y + coor_x*seven_cols];
        }
    }
    break;
    case 8:
    for(coor_y = 0; coor_y < eight_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - eight_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*4 - eight_rows;
        
	for (coor_x = eight_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = eight_bmpdata[coor_y + coor_x*eight_cols];
        }
    }
    break;
    case 9:
    for(coor_y = 0; coor_y < nine_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - nine_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*4 - nine_rows;
        
	for (coor_x = nine_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = nine_bmpdata[coor_y + coor_x*nine_cols];
        }
    }
    break;
    case 10:
    for(coor_y = 0; coor_y < ten_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - ten_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*4 - ten_rows;
        
	for (coor_x = ten_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = ten_bmpdata[coor_y + coor_x*ten_cols];
        }
    }
    break;
    default:
    break;
    }
 
    //display speed_digit
    switch(*player_speed)
    {
    case 1:	
    for(coor_y = 0; coor_y < one_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - one_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*3 - one_rows;
        
	for (coor_x = one_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = one_bmpdata[coor_y + coor_x*one_cols];
        }
    }
    break;
    case 2:
    for(coor_y = 0; coor_y < two_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - two_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*3 - two_rows;
        
	for (coor_x = two_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = two_bmpdata[coor_y + coor_x*two_cols];
        }
    }
    break;
    case 3:
    for(coor_y = 0; coor_y < three_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - three_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*3 - three_rows;
        
	for (coor_x = three_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = three_bmpdata[coor_y + coor_x*three_cols];
        }
    }
    break;
    case 4:
    for(coor_y = 0; coor_y < four_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - four_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*3 - four_rows;
        
	for (coor_x = four_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = four_bmpdata[coor_y + coor_x*four_cols];
        }
    }
    break;
    case 5:
    for(coor_y = 0; coor_y < five_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - five_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*3 - five_rows;
        
	for (coor_x = five_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = five_bmpdata[coor_y + coor_x*five_cols];
        }
    }
    break;
    case 6:
    for(coor_y = 0; coor_y < six_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - six_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*3 - six_rows;
        
	for (coor_x = six_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = six_bmpdata[coor_y + coor_x*six_cols];
        }
    }
    break;
    case 7:
    for(coor_y = 0; coor_y < seven_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - seven_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*3 - seven_rows;
        
	for (coor_x = seven_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = seven_bmpdata[coor_y + coor_x*seven_cols];
        }
    }
    break;
    case 8:
    for(coor_y = 0; coor_y < eight_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - eight_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*3 - eight_rows;
        
	for (coor_x = eight_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = eight_bmpdata[coor_y + coor_x*eight_cols];
        }
    }
    break;
    case 9:
    for(coor_y = 0; coor_y < nine_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - nine_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*3 - nine_rows;
        
	for (coor_x = nine_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = nine_bmpdata[coor_y + coor_x*nine_cols];
        }
    }
    break;
    case 10:
    for(coor_y = 0; coor_y < ten_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - ten_cols- 20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*3 - ten_rows;
        
	for (coor_x = ten_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = ten_bmpdata[coor_y + coor_x*ten_cols];
        }
    }
    break;
    default:
    break;
    }

    // display gender_image(woman/man)
    switch(*player_gender) {
    case 0:
    for(coor_y = 0; coor_y < woman_button_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height -woman_button_cols -20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*2 -woman_button_rows;
        
	for (coor_x = woman_button_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = woman_button_bmpdata[coor_y + coor_x*woman_button_cols];
        }
    }
    break;
    case 1:
    for(coor_y = 0; coor_y < man_button_cols; coor_y++) {
        ptr = (unsigned long*)pfbmap + screen_width * \
	      (coor_y+ screen_height - man_button_cols -20) + \
	      (screen_width-logo_rows-fix_button_rows-50)/3*2 -man_button_rows;
        
	for (coor_x = man_button_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = man_button_bmpdata[coor_y + coor_x*man_button_cols];
        }
    }
    break;
    default:
    break;
    }

    int logo_center_y = screen_height/2;
    int logo_start_x = screen_width - logo_rows;
    int fix_button_center_y = screen_height/2;
    int fix_button_start_x = 0;
    int fix_flag = 1;

    int volume_plus_center_y = screen_height/2 - plus_button_cols/2 -10;
    int volume_plus_start_x = (screen_width-logo_rows-fix_button_rows-50)/3*4 \
			      -25 -plus_button_rows;

    int volume_minus_center_y = screen_height/2 + minus_button_cols/2 + 10;
    int volume_minus_start_x = (screen_width-logo_rows-fix_button_rows-50)/3*4 \
			       -25 -minus_button_rows;

    int speed_plus_center_y = screen_height/2- plus_button_cols/2 -10;
    int speed_plus_start_x = (screen_width-logo_rows-fix_button_rows-50)/3*3 \
			     -25 -plus_button_rows;
    int speed_minus_center_y = screen_height/2 + minus_button_cols/2 +10;
    int speed_minus_start_x = (screen_width-logo_rows-fix_button_rows-50)/3*3 \
			      -25 -minus_button_rows;

    int change_center_y = screen_height/2;
    int change_start_x = (screen_width-logo_rows-fix_button_rows-50)/3*2 \
			 -25 -change_button_rows;

    // touch event wating loop!
    while(1)
    {
	    // read touch event & coordinates
	    readFirstCoordinate(touch_fp, &touch_x, &touch_y);
	    
	    // see which part is touched
	    // logo button part
	    if((touch_y >= logo_center_y-logo_cols/2) && \
	       (touch_y <= logo_center_y+logo_cols/2) && \
	       (touch_x >= logo_start_x) && \
	       (touch_x <= logo_start_x+logo_rows))
	    {
		    break;
				   
	    } // fix/unfix button part
	    else if((touch_y >= fix_button_center_y-fix_button_cols/2) && \
	       	    (touch_y <= fix_button_center_y+fix_button_cols/2) && \
	            (touch_x >= fix_button_start_x) && \
	            (touch_x <= fix_button_start_x+fix_button_rows))
	    {
		   if(fix_flag==1) {
		   	   arduino_serial_write("S");
			   fix_flag=0;
    			   // display unfix button
    			   // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    			   for(coor_y = 0; coor_y < unfix_button_cols; coor_y++) {
        		      ptr = (unsigned long*)pfbmap + screen_width * \
	      		            (coor_y+ screen_height/2 - unfix_button_cols/2);
        
			      for (coor_x = unfix_button_rows-1; coor_x >= 0; coor_x--) {
            			    *ptr++  = unfix_button_bmpdata[coor_y + coor_x*unfix_button_cols];
       			      }
    			   }
		   }
		   else {
			   arduino_serial_write("U");
			   fix_flag=1;
    			   // display fix button
    			   // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    			   for(coor_y = 0; coor_y < fix_button_cols; coor_y++) {
        		      ptr = (unsigned long*)pfbmap + screen_width * \
	      		            (coor_y+ screen_height/2 - fix_button_cols/2);
        
			      for (coor_x = fix_button_rows-1; coor_x >= 0; coor_x--) {
            			    *ptr++  = fix_button_bmpdata[coor_y + coor_x*fix_button_cols];
       			      }
		  	   }
	    	   }

		   sleep(2);
	     } // volume plus button part
	     else if((touch_y >= volume_plus_center_y-plus_button_cols/2) && \
	             (touch_y <= volume_plus_center_y+plus_button_cols/2) && \
	             (touch_x >= volume_plus_start_x) && \
	             (touch_x <= volume_plus_start_x+plus_button_rows))
	     {	     
		     printf("volume plus clicked!\n");

	     } // volume minus button part
	     else if((touch_y >= volume_minus_center_y-minus_button_cols/2) && \
	             (touch_y <= volume_minus_center_y+minus_button_cols/2) && \
	             (touch_x >= volume_minus_start_x) && \
	             (touch_x <= volume_minus_start_x+minus_button_rows))
	     {
		     printf("volume minus clicked!\n");

	     } // speed plus button part
	     else if((touch_y >= speed_plus_center_y-play_button_cols/2) && \
	             (touch_y <= speed_plus_center_y+play_button_cols/2) && \
	             (touch_x >= speed_plus_start_x) && \
	             (touch_x <= speed_plus_start_x+play_button_rows))
	     {
		     printf("speed plus clicked!\n");

	     } // speed minus button part
	     else if((touch_y >= speed_minus_center_y-minus_button_cols/2) && \
	             (touch_y <= speed_minus_center_y+minus_button_cols/2) && \
	             (touch_x >= speed_minus_start_x) && \
	             (touch_x <= speed_minus_start_x+minus_button_rows))
	     {
		     printf("speed minus clicked!\n");

	     } // change button part
	     else if((touch_y >= change_center_y-change_button_cols/2) && \
	             (touch_y <= change_center_y+change_button_cols/2) && \
	             (touch_x >= change_start_x) && \
	             (touch_x <= change_start_x+change_button_rows))
	     {
		     printf("change clicked!\n");

	     }
	    
    }
}

void showPlayMenu(int player_volume, int player_speed, int player_gender)
{
    // touch variables
    char eventFullPathName[100]= EVENT_STR;
    int touch_x,touch_y, touch_prex=0, touch_prey=0;

    // frame variables	
    int i, j, k, t;
    int coor_x, coor_y;
    int logo_cols=0, logo_rows=0;
    int play_button_cols=0, play_button_rows=0;
    int setting_button_cols=0, setting_button_rows=0;
    int play_cols=0, play_rows;
    int first_page_cols=0, first_page_rows=0;
    int second_page_cols=0, second_page_rows=0;

    char *logo_pData, *logo_data;
    char logo_name[200] = "./ui_images/logo_play.bmp";
    char *play_button_pData, *play_button_data;
    char play_button_name[200] = "./ui_images/play_button.bmp";
    char *setting_button_pData, *setting_button_data;
    char setting_button_name[200] = "./ui_images/setting_button.bmp";
    char *play_pData, *play_data;
    char play_name[200] = "./ui_images/play.bmp";

    char *first_page_pData, *first_page_data;
    char *second_page_pData, *second_page_data;
    
    char book_name[100] = "book";
    char pages_dir[100] = "book_pages";

    unsigned long logo_bmpdata[640*800];
    unsigned long logo_pixel;
    unsigned long play_bmpdata[800*800];
    unsigned long play_pixel;

    unsigned long *ptr;
    char r,g,b;

    // read logo bitmap file
    read_bmp(logo_name, &logo_pData, &logo_data, &logo_cols, &logo_rows);
    
    // get logo bitmap data
    for(j = 0; j < logo_rows; j++)
    {
        k   =   j * logo_cols * 3;
        t   =   (logo_rows - 1 - j) * logo_cols;

        for(i = 0; i < logo_cols; i++)
        {
            b   =   *(logo_data + (k + i * 3));
            g   =   *(logo_data + (k + i * 3 + 1));
            r   =   *(logo_data + (k + i * 3 + 2));

            logo_pixel = ((r<<16) | (g<<8) | b);
            logo_bmpdata[t+i] = logo_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&logo_pData);

    // read play picture bitmap file
    read_bmp(play_name, &play_pData, \
		    &play_data, &play_cols, &play_rows);
    
    // get play picture bitmap data
    for(j = 0; j < play_rows; j++)
    {
        k   =   j * play_cols * 3;
        t   =   (play_rows - 1 - j) * play_cols;

        for(i = 0; i < play_cols; i++)
        {
            b   =   *(play_data + (k + i * 3));
            g   =   *(play_data + (k + i * 3 + 1));
            r   =   *(play_data + (k + i * 3 + 2));

            play_pixel = ((r<<16) | (g<<8) | b);
            play_bmpdata[t+i] = play_pixel;          
	    // save bitmap data bottom up
        }
    }

    close_bmp(&play_pData);

    // fb background clear 
    for(coor_y = 0; coor_y < screen_height; coor_y++) {
        ptr =   (unsigned long *)pfbmap + (screen_width * coor_y);
       
	// logo below background
	for(coor_x = 0; coor_x < screen_width-logo_rows-2; coor_x++)
        {
            *ptr++  =   0xffffff;
        }

	// logo part background
	for(coor_x = screen_width-logo_rows-2; coor_x < screen_width; coor_x++)
        {
            *ptr++  =   0xaaaaaa;
        }
    }

    // display logo
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < logo_cols; coor_y++) {
	if(logo_cols%2 != 0) {
       	   ptr = (unsigned long*)pfbmap + screen_width*(coor_y+screen_height/2-logo_cols/2-1) \
		   + screen_width-logo_rows;
	}
	else {
           ptr = (unsigned long*)pfbmap + screen_width*(coor_y+screen_height/2-logo_cols/2) \
		   + screen_width-logo_rows;
        }
	for (coor_x = logo_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = logo_bmpdata[coor_y + coor_x*logo_cols];
        }
    }

    // display play picture
    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    for(coor_y = 0; coor_y < play_cols; coor_y++) {
	if(play_cols%2 != 0) {
       	   ptr = (unsigned long*)pfbmap + screen_width*(coor_y+screen_height/2-play_cols/2-1) \
		   + (screen_width-logo_rows)/2 - play_rows/2;
	}
	else {
           ptr = (unsigned long*)pfbmap + screen_width*(coor_y) \
		   + (screen_width-logo_rows)/2 - play_rows/2;
        }
	for (coor_x = play_rows-1; coor_x >= 0; coor_x--) {
            *ptr++  = play_bmpdata[coor_y + coor_x*play_cols];
        }
    }

    int logo_center_y = screen_height/2;
    int logo_start_x = screen_width - logo_rows;
    
    int first_page_number = 1;
    int seven_segs_page_number = 0;

    char first_page_name[100];

    char first_display_name[100] = "./book_pages/first_page_display.bmp";

    char mp3_save_dir[100] = "./book_mp3s/";
    char first_page_mp3_full_name[200];


    // initialize seven segments device
    // page number is set to 0
    if(seven_seg_init_flag == 0)
    {
    	initPage(&seven_segs_page_number);
	seven_seg_init_flag = 1;
    }

    // touch event wating loop!
    while(1)
    {
	    int error_flag = 0;
	    char* first_page_text_pointer = "";

	    // read touch event & coordinates
	    readFirstCoordinate(touch_fp, &touch_x, &touch_y);
	    touch_prex = touch_x;
	    touch_prey = touch_y;
	    
	    // see which part is touched
	    // logo button part
	    if((touch_y >= logo_center_y-logo_cols/2) && \
	       (touch_y <= logo_center_y+logo_cols/2) && \
	       (touch_x >= logo_start_x) && \
	       (touch_x <= logo_start_x+logo_rows))
	    {
		    break;	   
	    }

	    sprintf(first_page_name, "%s/%s_page_%d.bmp", pages_dir, \
			book_name, first_page_number);

	    /*
	       Set Color Led to Process State
	     */
	    setColorLed("process");

	    /*
	      Capture pages!
	     */
	    capture_page(first_page_name);

	    /*
	    // display pages
      	    read_bmp(first_display_name, &first_page_pData, &first_page_data, \
			    &first_page_cols, &first_page_rows);

	    printf("display Start\n\n");

    	    // get first page bitmap data
    	    for(j = 0; j < first_page_rows; j++)
    	    {
        	    k   =   j * first_page_cols * 3;
        	    t   =   (first_page_rows - 1 - j) * first_page_cols;

        	    for(i = 0; i < first_page_cols; i++)
        	    {
            	    	b   =   *(first_page_data + (k + i * 3));
            	 	g   =   *(first_page_data + (k + i * 3 + 1));
            	        r   =   *(first_page_data + (k + i * 3 + 2));

            	        first_page_pixel = ((r<<16) | (g<<8) | b);
            		first_page_bmpdata[t+i] = first_page_pixel; 
			// save bitmap data bottom up
        	    }
    	    }

    	    close_bmp(&first_page_pData);

	    printf("Display Done\n\n");

    	    // display first page
    	    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    	    for(coor_y = 0; coor_y < first_page_cols; coor_y++) {
	 	    if(first_page_cols%2 != 0) {
       	   	    	ptr = (unsigned long*)pfbmap + screen_width * \
		 	     (coor_y+screen_height/2 -first_page_cols/2) + \
		 	     (screen_width-logo_rows)/6*5 - first_page_rows;
		    }
		    else {
           	    	ptr = (unsigned long*)pfbmap + screen_width * \
	   	 	      (coor_y+screen_height/2 -first_page_cols/2) + \
		 	      (screen_width-logo_rows)/6*5 - first_page_rows;
        	    }
		    for (coor_x = first_page_rows-1; coor_x >= 0; coor_x--) {
            	    	*ptr++  = first_page_bmpdata[coor_y + coor_x*first_page_cols];
        	    }
   	    } 
	    */

	    /*

      	    read_bmp(second_display_name, &second_page_pData, &second_page_data, \
			    &second_page_cols, &second_page_rows);
   
    	    // get second page bitmap data
    	    for(j = 0; j < second_page_rows; j++)
    	    {
        	    k   =   j * second_page_cols * 3;
        	    t   =   (second_page_rows - 1 - j) * second_page_cols;

        	    for(i = 0; i < second_page_cols; i++)
        	    {
            	    	b   =   *(second_page_data + (k + i * 3));
            	 	g   =   *(second_page_data + (k + i * 3 + 1));
            	        r   =   *(second_page_data + (k + i * 3 + 2));

            	        second_page_pixel = ((r<<16) | (g<<8) | b);
            		second_page_bmpdata[t+i] = second_page_pixel; 
			// save bitmap data bottom up
        	    }
    	    }

    	    close_bmp(&second_page_pData);

    	    // display second page
    	    // direction for image generating : (0,0)-> (1,0)-> (2,0)-> ...-> (row, column)
    	    for(coor_y = 0; coor_y < second_page_cols; coor_y++) {
	 	    if(second_page_cols%2 != 0) {
       	   	    	ptr = (unsigned long*)pfbmap + screen_width * \
		 	  (coor_y+screen_height/2+20) + \
		 	  (screen_width-logo_rows)/6*5 - second_page_rows;
		    }
		    else {
           	    	ptr = (unsigned long*)pfbmap + screen_width * \
	   	 	  (coor_y+screen_height/2+20) + \
		 	  (screen_width-logo_rows)/6*5 - second_page_rows;
        	    }
		    for (coor_x = second_page_rows-1; coor_x >= 0; coor_x--) {
		    	*ptr++  = second_page_bmpdata[coor_y + coor_x*second_page_cols];
        	    }
   	    } 
	    */

	    /*
		Call Cloud Vision and make Texts!!	
	     */
	    first_page_text_pointer = (char*) callGoogleVision(first_page_name);
	    if(strcmp(first_page_text_pointer, "no response") == 0)
	    {
		error_flag = 1;
	    }
	
	    printf("\n\n---Return Text----:\n%s\n\n", first_page_text_pointer);

	    /*
		Call Speech Synthesis and make Mp3s!!
	    */
	    sprintf(first_page_mp3_full_name, "%spage_%d.mp3", \
			    mp3_save_dir, first_page_number);

	    if(error_flag != 1)
	    {
	    	callSpeechSynthesis(first_page_text_pointer, \
				    first_page_mp3_full_name, first_page_number, \
				    player_gender, player_speed);
	    }
	    else
	    {
		printf("No Text!!\n");
	    }

	    /*
		Play Mp3 Files
	     */
	    // set page number of seven segments 
	    //setNextPage(&seven_segs_page_number);
	    
	    // Set Dotmatrix to play mode
	    setDotMatrix(100);

	    // Set Color Led to Read State
	    setColorLed("read");

	    // play first page mp3 file
	    if(error_flag != 1)
	    {
	    	playMp3File(first_page_mp3_full_name);
	    }
	    
	    // set page number of seven segments
	    //setNextPage(&seven_segs_page_number);

	    // set Dotmatrix to volume display mode
	    setDotMatrix(player_volume);

	    // set Color Led to Wait State
	    setColorLed("wait");

	    /*
	    	Turn the page!!
	    */
	    arduino_serial_write("T");

	    /*
		Wait until turing done

		return value is 0 if arduino sends "Done"
		otherwise return value is 1
	     */
	    //while(arduino_serial_read()==1) {
	    //
	    //}
            
	    sleep(5);

	    // increase page number
	    if(error_flag != 1) first_page_number = first_page_number + 2;

	    /* 
	       Check Stop Option From Dipswitch

	       return value is 0x1~0xF for stopping
	       otherwise it is 0
	     */
	    // when stopping
	    if(dipswitch_read() != 0) {
		break;
	    }
	    // otherwise keep going
    }
}
