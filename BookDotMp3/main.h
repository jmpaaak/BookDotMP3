// main.h

// frame buffer define
#define FBDEV_FILE "/dev/fb0"
#define BIT_VALUE_24BIT 24

// touch define
#define INPUT_DEVICE_LIST "/proc/bus/input/devices"
#define EVENT_STR "/dev/input/event2"
#define MAX_BUFF 200
#define MAX_TOUCH_X 0x740
#define MAX_TOUCH_Y 0x540

// frame buffer functions
void read_bmp(char *filename, char **pDib, char **data, \
	      int *cols, int *rows);
void close_bmp(char **pDib); // DIB(Device Independent Bitmap)

// touch functions
void readFirstCoordinate(int fd, int* cx, int* cy);

// menu functions
void showSettingMenu(int *player_volume, int *player_speed, \
		     int *player_gender);
void showPlayMenu(int player_volume, int player_speed, int player_gender);
