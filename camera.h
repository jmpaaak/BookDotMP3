// camera.h
#define V4L2_CID_CACHEABLE (V4L2_CID_BASE+40)
#define FBDEV_FILE "/dev/fb0"

#define CHECK(return_value)                                          \
    if (return_value < 0) {                                          \
        printf("%s::%d fail. errno: %s, m_camera_id = %d\n",             \
             __func__, __LINE__, strerror(errno), 0);      \
        return -1;                                                   \
    }

#define CHECK_PTR(return_value)                                      \
    if (return_value < 0) {                                          \
        printf("%s::%d fail, errno: %s, m_camera_id = %d\n",             \
             __func__,__LINE__, strerror(errno), 0);       \
        return NULL;                                                 \
    }


#define CAMERA_PREVIEW_WIDTH       640
#define CAMERA_PREVIEW_HEIGHT      480
#define MAX_BUFFERS     8

#define MAX_PLANES      (1)
#define V4L2_BUF_TYPE				V4L2_BUF_TYPE_VIDEO_CAPTURE

#define PREVIEW_NUM_PLANE (1)


#define V4L2_MEMORY_TYPE V4L2_MEMORY_MMAP

#define CAMERA_DEV_NAME   "/dev/video0"
#define PREVIEW_MODE 1

enum scenario_mode {
    IS_MODE_PREVIEW_STILL,
    IS_MODE_PREVIEW_VIDEO,
    IS_MODE_CAPTURE_STILL,
    IS_MODE_CAPTURE_VIDEO,
    IS_MODE_MAX
};

extern int screen_width;
extern int screen_height;
extern int bits_per_pixel;
extern int line_length;
extern int fbfd;
extern int mem_size;
extern unsigned char *pfbmap;

void capture_page(char* first_page_name, char* second_page_name);
void makeBitmap(FILE *first_page_name, FILE *second_page_name, \
		unsigned char *viedoFrame, int videoWidth, int videoHeight);
