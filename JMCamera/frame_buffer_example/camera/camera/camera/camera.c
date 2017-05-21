#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // for open/close
#include <fcntl.h>      // for O_RDWR
#include <sys/ioctl.h>  // for ioctl
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <sys/poll.h>
#include <linux/fb.h>
#include <linux/videodev2.h>
#include "videodev2.h"
#include "SecBuffer.h"
#include "camera.h"



#define V4L2_CID_CACHEABLE                      (V4L2_CID_BASE+40)

/*
V4L2_PIX_FMT_NV12:
V4L2_PIX_FMT_NV12T:
V4L2_PIX_FMT_NV21:
V4L2_PIX_FMT_YUV420:
V4L2_PIX_FMT_YVU420:
V4L2_PIX_FMT_YVU420M:
V4L2_PIX_FMT_RGB565:
V4L2_PIX_FMT_YUYV:
V4L2_PIX_FMT_YVYU:
V4L2_PIX_FMT_UYVY:
V4L2_PIX_FMT_VYUY:
V4L2_PIX_FMT_NV16:
V4L2_PIX_FMT_NV61:
V4L2_PIX_FMT_YUV422P:
*/

static int m_preview_v4lformat = V4L2_PIX_FMT_RGB565;
//static int m_preview_v4lformat = V4L2_PIX_FMT_YUV422P;
static int  m_cam_fd;
static struct SecBuffer m_buffers_preview[MAX_BUFFERS];
static struct pollfd   m_events_c;
static int   screen_width;
static int   screen_height;
static int   bits_per_pixel;
static int   line_length;

static int close_buffers(struct SecBuffer *buffers, int num_of_buf)
{
	int ret,i,j;

	for ( i = 0; i < num_of_buf; i++) {
		for( j = 0; j < MAX_PLANES; j++) {
			if (buffers[i].virt.extP[j]) {
				ret = munmap(buffers[i].virt.extP[j], buffers[i].size.extS[j]);
				buffers[i].virt.extP[j] = NULL;
			}
		}
	}

	return 0;
}

static int get_pixel_depth(unsigned int fmt)
{
	int depth = 0;

	switch (fmt) {
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_NV21:
	case V4L2_PIX_FMT_YUV420:
	case V4L2_PIX_FMT_YVU420:
		depth = 12;
		break;

	case V4L2_PIX_FMT_RGB565:
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_YVYU:
	case V4L2_PIX_FMT_UYVY:
	case V4L2_PIX_FMT_VYUY:
	case V4L2_PIX_FMT_NV16:
	case V4L2_PIX_FMT_NV61:
	case V4L2_PIX_FMT_YUV422P:
		depth = 16;
		break;

	case V4L2_PIX_FMT_RGB32:
		depth = 32;
		break;
	}

	return depth;
}


static int fimc_poll(struct pollfd *events)
{
	int ret;

	/* 10 second delay is because sensor can take a long time
	* to do auto focus and capture in dark settings
	*/
	ret = poll(events, 1, 10000);
	if (ret < 0) {
		printf("ERR(%s):poll error\n", __func__);
		return ret;
	}

	if (ret == 0) {
		printf("ERR(%s):No data in 10 secs..\n", __func__);
		return ret;
	}

	return ret;
}

static int fimc_v4l2_querycap(int fp)
{
	struct v4l2_capability cap;

	if (ioctl(fp, VIDIOC_QUERYCAP, &cap) < 0) {
		printf("ERR(%s):VIDIOC_QUERYCAP failed\n", __func__);
		return -1;
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		printf("ERR(%s):no capture devices\n", __func__);
		return -1;
	}

	return 0;
	}

static const __u8* fimc_v4l2_enuminput(int fp, int index)
{
	static struct v4l2_input input;

	input.index = index;
	if (ioctl(fp, VIDIOC_ENUMINPUT, &input) != 0) {
		printf("ERR(%s):No matching index found\n", __func__);
		return NULL;
	}
	printf("Name of input channel[%d] is %s\n", input.index, input.name);

	return input.name;
}

static int fimc_v4l2_s_input(int fp, int index)
{
	struct v4l2_input input;

	input.index = index;

	if (ioctl(fp, VIDIOC_S_INPUT, &input) < 0) {
		printf("ERR(%s):VIDIOC_S_INPUT failed\n", __func__);
		return -1;
	}

	return 0;
}

static int fimc_v4l2_s_fmt(int fp, int width, int height, unsigned int fmt, enum v4l2_field field, unsigned int num_plane)
{
	struct v4l2_format v4l2_fmt;
	struct v4l2_pix_format pixfmt;
	//unsigned int framesize;

	memset(&v4l2_fmt, 0, sizeof(struct v4l2_format));
	v4l2_fmt.type = V4L2_BUF_TYPE;


	memset(&pixfmt, 0, sizeof(pixfmt));

	pixfmt.width = width;
	pixfmt.height = height;
	pixfmt.pixelformat = fmt;
	pixfmt.field = V4L2_FIELD_NONE;

	v4l2_fmt.fmt.pix = pixfmt;
	printf("fimc_v4l2_s_fmt : width(%d) height(%d)\n", width, height);

	/* Set up for capture */
	if (ioctl(fp, VIDIOC_S_FMT, &v4l2_fmt) < 0) {
		printf("ERR(%s):VIDIOC_S_FMT failed\n", __func__);
		return -1;
	}

	return 0;
}

static int fimc_v4l2_s_fmt_cap(int fp, int width, int height, unsigned int fmt)
{
	struct v4l2_format v4l2_fmt;
	struct v4l2_pix_format pixfmt;

	memset(&pixfmt, 0, sizeof(pixfmt));

	v4l2_fmt.type = V4L2_BUF_TYPE;

	pixfmt.width = width;
	pixfmt.height = height;
	pixfmt.pixelformat = fmt;
	if (fmt == V4L2_PIX_FMT_JPEG)
	pixfmt.colorspace = V4L2_COLORSPACE_JPEG;

	v4l2_fmt.fmt.pix = pixfmt;
	printf("fimc_v4l2_s_fmt_cap : width(%d) height(%d)\n", width, height);

	/* Set up for capture */
	if (ioctl(fp, VIDIOC_S_FMT, &v4l2_fmt) < 0) {
		printf("ERR(%s):VIDIOC_S_FMT failed\n", __func__);
		return -1;
	}

	return 0;
}

int fimc_v4l2_s_fmt_is(int fp, int width, int height, unsigned int fmt, enum v4l2_field field)
{
	struct v4l2_format v4l2_fmt;
	struct v4l2_pix_format pixfmt;

	memset(&pixfmt, 0, sizeof(pixfmt));

	v4l2_fmt.type = V4L2_BUF_TYPE_PRIVATE;

	pixfmt.width = width;
	pixfmt.height = height;
	pixfmt.pixelformat = fmt;
	pixfmt.field = field;

	v4l2_fmt.fmt.pix = pixfmt;
	printf("fimc_v4l2_s_fmt_is : width(%d) height(%d)\n", width, height);

	/* Set up for capture */
	if (ioctl(fp, VIDIOC_S_FMT, &v4l2_fmt) < 0) {
		printf("ERR(%s):VIDIOC_S_FMT failed\n", __func__);
		return -1;
	}

	return 0;
}

static int fimc_v4l2_enum_fmt(int fp, unsigned int fmt)
{
	struct v4l2_fmtdesc fmtdesc;
	int found = 0;

	fmtdesc.type = V4L2_BUF_TYPE;
	fmtdesc.index = 0;

	while (ioctl(fp, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
		if (fmtdesc.pixelformat == fmt) {
			printf("passed fmt = %#x found pixel format[%d]: %s\n", fmt, fmtdesc.index, fmtdesc.description);
			found = 1;
			break;
		}

		fmtdesc.index++;
	}

	if (!found) {
		printf("unsupported pixel format\n");
		return -1;
	}

	return 0;
}

static int fimc_v4l2_reqbufs(int fp, enum v4l2_buf_type type, int nr_bufs)
{
	struct v4l2_requestbuffers req;

	req.count = nr_bufs;
	req.type = type;
	req.memory = V4L2_MEMORY_TYPE;

	if (ioctl(fp, VIDIOC_REQBUFS, &req) < 0) {
		printf("ERR(%s):VIDIOC_REQBUFS failed\n", __func__);
		return -1;
	}

	return req.count;
}

static int fimc_v4l2_querybuf(int fp, struct SecBuffer *buffers, enum v4l2_buf_type type, int nr_frames, int num_plane)
{
	struct v4l2_buffer v4l2_buf;

	int i, ret ;

	for (i = 0; i < nr_frames; i++) {
		v4l2_buf.type = type;
		v4l2_buf.memory = V4L2_MEMORY_TYPE;
		v4l2_buf.index = i;

		ret = ioctl(fp, VIDIOC_QUERYBUF, &v4l2_buf); // query video buffer status
		if (ret < 0) {
			printf("ERR(%s):VIDIOC_QUERYBUF failed\n", __func__);
			return -1;
		}

		buffers[i].size.s = v4l2_buf.length;

		if ((buffers[i].virt.p = (char *)mmap(0, v4l2_buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
			fp, v4l2_buf.m.offset)) < 0) {
			printf("%s %d] mmap() failed",__func__, __LINE__);
			return -1;
		}
		printf("buffers[%d].virt.p = %p v4l2_buf.length = %d\n", i, buffers[i].virt.p, v4l2_buf.length);
	}
	return 0;
}

static int fimc_v4l2_streamon(int fp)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE;
	int ret;

	ret = ioctl(fp, VIDIOC_STREAMON, &type);
	if (ret < 0) {
		printf("ERR(%s):VIDIOC_STREAMON failed\n", __func__);
		return ret;
	}

	return ret;
}

static int fimc_v4l2_streamoff(int fp)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE;
	int ret;

	printf("%s :", __func__);
	ret = ioctl(fp, VIDIOC_STREAMOFF, &type);
	if (ret < 0) {
		printf("ERR(%s):VIDIOC_STREAMOFF failed\n", __func__);
		return ret;
	}

	return ret;
}

static int fimc_v4l2_qbuf(int fp, int index )
{
	struct v4l2_buffer v4l2_buf;
	int ret;

	v4l2_buf.type = V4L2_BUF_TYPE;
	v4l2_buf.memory = V4L2_MEMORY_TYPE;
	v4l2_buf.index = index;


	ret = ioctl(fp, VIDIOC_QBUF, &v4l2_buf);
	if (ret < 0) {
		printf("ERR(%s):VIDIOC_QBUF failed\n", __func__);
		return ret;
	}

	return 0;
}

static int fimc_v4l2_dqbuf(int fp, int num_plane)
{
	struct v4l2_buffer v4l2_buf;
	int ret;

	v4l2_buf.type = V4L2_BUF_TYPE;
	v4l2_buf.memory = V4L2_MEMORY_TYPE;

	ret = ioctl(fp, VIDIOC_DQBUF, &v4l2_buf);
	if (ret < 0) {
		printf("ERR(%s):VIDIOC_DQBUF failed, dropped frame\n", __func__);
		return ret;
	}

	return v4l2_buf.index;
}

static int fimc_v4l2_g_ctrl(int fp, unsigned int id)
{
	struct v4l2_control ctrl;
	int ret;

	ctrl.id = id;

	ret = ioctl(fp, VIDIOC_G_CTRL, &ctrl);
	if (ret < 0) {
		printf("ERR(%s): VIDIOC_G_CTRL(id = 0x%x (%d)) failed, ret = %d\n",
			 __func__, id, id-V4L2_CID_PRIVATE_BASE, ret);
		return ret;
	}

	return ctrl.value;
}

static int fimc_v4l2_s_ctrl(int fp, unsigned int id, unsigned int value)
{
	struct v4l2_control ctrl;
	int ret;

	ctrl.id = id;
	ctrl.value = value;

	ret = ioctl(fp, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0) {
		printf("ERR(%s):VIDIOC_S_CTRL(id = %#x (%d), value = %d) failed ret = %d\n",
		 __func__, id, id-V4L2_CID_PRIVATE_BASE, value, ret);

		return ret;
	}

	return ctrl.value;
}

static int fimc_v4l2_s_ext_ctrl(int fp, unsigned int id, void *value)
{
	struct v4l2_ext_controls ctrls;
	struct v4l2_ext_control ctrl;
	int ret;

	ctrl.id = id;

	ctrls.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
	ctrls.count = 1;
	ctrls.controls = &ctrl;

	ret = ioctl(fp, VIDIOC_S_EXT_CTRLS, &ctrls);
	if (ret < 0)
		printf("ERR(%s):VIDIOC_S_EXT_CTRLS failed\n", __func__);

	return ret;
}



static int fimc_v4l2_g_parm(int fp, struct v4l2_streamparm *streamparm)
{
	int ret;

	streamparm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(fp, VIDIOC_G_PARM, streamparm);
	if (ret < 0) {
		printf("ERR(%s):VIDIOC_G_PARM failed\n", __func__);
		return -1;
	}

	printf("%s : timeperframe: numerator %d, denominator %d\n", __func__,
			streamparm->parm.capture.timeperframe.numerator,
			streamparm->parm.capture.timeperframe.denominator);

	return 0;
}

static int fimc_v4l2_s_parm(int fp, struct v4l2_streamparm *streamparm)
{
	int ret;

	streamparm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(fp, VIDIOC_S_PARM, streamparm);
	if (ret < 0) {
		printf("ERR(%s):VIDIOC_S_PARM failed\n", __func__);
		return ret;
	}

	return 0;
}

int CreateCamera(int index)
{
	printf("%s :\n", __func__);
	int ret = 0;

	m_cam_fd = open(CAMERA_DEV_NAME, O_RDWR);
	if (m_cam_fd < 0) {
		printf("ERR(%s):Cannot open %s (error : %s)\n", __func__, CAMERA_DEV_NAME, strerror(errno));
		return -1;
	}
	printf("%s: open(%s) --> m_cam_fd %d\n", __func__, CAMERA_DEV_NAME, m_cam_fd);

	ret = fimc_v4l2_querycap(m_cam_fd);
	CHECK(ret);
	if (!fimc_v4l2_enuminput(m_cam_fd, index)) {
		printf("m_cam_fd(%d) fimc_v4l2_enuminput fail\n", m_cam_fd);
		return -1;
	}
	ret = fimc_v4l2_s_input(m_cam_fd, index);
	CHECK(ret);


	return 0;
}
void DestroyCamera()
{
	if (m_cam_fd > -1) {
		close(m_cam_fd);
		m_cam_fd = -1;
	}

}

int startPreview(void)
{
	int i;
	//v4l2_streamparm streamparm;
	printf("%s :\n", __func__);

	if (m_cam_fd <= 0) {
		printf("ERR(%s):Camera was closed\n", __func__);
		return -1;
	}

	memset(&m_events_c, 0, sizeof(m_events_c));
	m_events_c.fd = m_cam_fd;
	m_events_c.events = POLLIN | POLLERR;

	/* enum_fmt, s_fmt sample */
	int ret = fimc_v4l2_enum_fmt(m_cam_fd,m_preview_v4lformat);
	CHECK(ret);

	ret = fimc_v4l2_s_fmt(m_cam_fd, CAMERA_PREVIEW_WIDTH, CAMERA_PREVIEW_HEIGHT, m_preview_v4lformat, V4L2_FIELD_ANY, PREVIEW_NUM_PLANE);
	CHECK(ret);

	fimc_v4l2_s_fmt_is(m_cam_fd, CAMERA_PREVIEW_WIDTH, CAMERA_PREVIEW_HEIGHT,
		m_preview_v4lformat, (enum v4l2_field) IS_MODE_PREVIEW_STILL);

	CHECK(ret);

	ret = fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CACHEABLE, 1);
	CHECK(ret);

	ret = fimc_v4l2_reqbufs(m_cam_fd, V4L2_BUF_TYPE, MAX_BUFFERS);
	CHECK(ret);

	ret = fimc_v4l2_querybuf(m_cam_fd, m_buffers_preview, V4L2_BUF_TYPE, MAX_BUFFERS, PREVIEW_NUM_PLANE);
	CHECK(ret);

	/* start with all buffers in queue  to capturer video */
	for (i = 0; i < MAX_BUFFERS; i++) {
		ret = fimc_v4l2_qbuf(m_cam_fd,  i);
		CHECK(ret);
	}

	ret = fimc_v4l2_streamon(m_cam_fd);
	CHECK(ret);

	printf("%s: got the first frame of the preview\n", __func__);

	return 0;
}

int stopPreview(void)
{
	int ret;

	if (m_cam_fd <= 0) {
		printf("ERR(%s):Camera was closed\n", __func__);
		return -1;
	}

	ret = fimc_v4l2_streamoff(m_cam_fd);
	CHECK(ret);

	close_buffers(m_buffers_preview, MAX_BUFFERS);

	fimc_v4l2_reqbufs(m_cam_fd, V4L2_BUF_TYPE, 0);

	return ret;
}

void initScreen(unsigned char *fb_mem )
{
	int         coor_y;
	int         coor_x;
	unsigned long *ptr;

	for(coor_y = 0; coor_y < screen_height; coor_y++)
	{
		ptr =   (unsigned long *)fb_mem + screen_width * coor_y ;
		for(coor_x = 0; coor_x < screen_width; coor_x++)
		{
			*ptr++  =   0x00000000;
		}
	}
}
static void yuv_a_rgb(unsigned char y, unsigned char u, unsigned char v,
               unsigned char* r, unsigned char* g, unsigned char* b)
{
	int amp=250;
	double R,G,B;

	R=amp*(0.004565*y+0.000001*u+0.006250*v-0.872);
	G=amp*(0.004565*y-0.001542*u-0.003183*v+0.531);
	B=amp*(0.004565*y+0.007935*u+/*0.000000*v*/-1.088);
	//printf("\nR = %f   G = %f   B = %f", R, G, B);

	if (R < 0)
		R=0;
	if (G < 0)
		G=0;
	if (B < 0)
		B=0;

	if (R > 255)
		R=255;
	if (G > 255)
		G=255;
	if (B > 255)
		B=255;

	*r=(unsigned char)(R);
	*g=(unsigned char)(G);
	*b=(unsigned char)(B);

}

static void Draw(unsigned char *displayFrame, unsigned char *videoFrame,int videoWidth, int videoHeight, \
				 int dFrameWidth,int dFrameHeight)
{ 
	int    x,y;
	unsigned char *offsetU;
	unsigned char *offsetV;
	unsigned char Y,U,V;
	unsigned char R,G,B;
	int lineLeng ;
	lineLeng = dFrameWidth*4;

	offsetV = videoFrame + videoWidth*videoHeight;
	offsetU = videoFrame + videoWidth*videoHeight + videoWidth*videoHeight/2;

	for ( y = 0 ; y < videoHeight ; y++ )
	{
		for(x = 0; x < videoWidth ; x++ )
		{
			Y = *(videoFrame + x + y*videoWidth);
			U = *(offsetU + (x + y*videoWidth)/2);
			V = *(offsetV + (x + y*videoWidth)/2);

			yuv_a_rgb(Y, U, V, &R, &G, &B);

			displayFrame[y*lineLeng + x*4 + 0] = R;
			displayFrame[y*lineLeng + x*4 + 1] = G;
			displayFrame[y*lineLeng + x*4 + 2] = B;
		}
	}
}
static void DrawFromRGB565(unsigned char *displayFrame, unsigned char *videoFrame,int videoWidth, int videoHeight, \
				 int dFrameWidth,int dFrameHeight)
{ 
	int    x,y;
	int lineLeng ;
	unsigned short *videptrTemp;
	unsigned short *videoptr = videoFrame;
	int temp;
	lineLeng = dFrameWidth*4;

	for ( y = 0 ; y < videoHeight ; y++ )
	{
		for(x = 0; x < videoWidth ;)
		{

			videptrTemp =  videoptr + videoWidth*y + x ;
			temp = y*lineLeng + x*4;
			displayFrame[temp + 2] = (unsigned char)((*videptrTemp & 0xF800) >> 8)  ;
			displayFrame[temp + 1] = (unsigned char)((*videptrTemp & 0x07E0) >> 3)  ;
			displayFrame[temp + 0] = (unsigned char)((*videptrTemp & 0x001F) << 3)  ;

			videptrTemp++;
			temp +=4;
			displayFrame[temp + 2] = (unsigned char)((*videptrTemp & 0xF800) >> 8)  ;
			displayFrame[temp + 1] = (unsigned char)((*videptrTemp & 0x07E0) >> 3)  ;
			displayFrame[temp + 0] = (unsigned char)((*videptrTemp & 0x001F) << 3)  ;

			videptrTemp++;
			temp +=4;
			displayFrame[temp + 2] = (unsigned char)((*videptrTemp & 0xF800) >> 8)  ;
			displayFrame[temp + 1] = (unsigned char)((*videptrTemp & 0x07E0) >> 3)  ;
			displayFrame[temp + 0] = (unsigned char)((*videptrTemp & 0x001F) << 3)  ;

			videptrTemp++;
			temp +=4;
			displayFrame[temp + 2] = (unsigned char)((*videptrTemp & 0xF800) >> 8)  ;
			displayFrame[temp + 1] = (unsigned char)((*videptrTemp & 0x07E0) >> 3)  ;
			displayFrame[temp + 0] = (unsigned char)((*videptrTemp & 0x001F) << 3)  ;
			x+=4;
		}
	}
}

#define  FBDEV_FILE "/dev/fb0"

int main(int argc, char **argv)
{
	int     fb_fd;
	int	    ret;
	int     index;


	struct  fb_var_screeninfo fbvar;
	struct  fb_fix_screeninfo fbfix;
	unsigned char   *fb_mapped;
	int             mem_size;


	if( access(FBDEV_FILE, F_OK) )
	{
		printf("%s: access error\n", FBDEV_FILE);
		return 1;
	}

	if( (fb_fd = open(FBDEV_FILE, O_RDWR)) < 0)
	{
		printf("%s: open error\n", FBDEV_FILE);
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

	CreateCamera(0);
	startPreview();

	while(1)
	{
		ret = fimc_poll(&m_events_c);
		CHECK_PTR(ret);
		index = fimc_v4l2_dqbuf(m_cam_fd, 1);
		//Draw(fb_mapped, m_buffers_preview[index].virt.p,CAMERA_PREVIEW_WIDTH,\
		//	 CAMERA_PREVIEW_HEIGHT,screen_width,screen_height);

		DrawFromRGB565(fb_mapped, m_buffers_preview[index].virt.p,CAMERA_PREVIEW_WIDTH,\
		CAMERA_PREVIEW_HEIGHT,screen_width,screen_height);
		//printf("index:%d\n",index);
		//printf("virt.p:%p, size.s:%d\n",m_buffers_preview[index].virt.p,\
		//	m_buffers_preview[index].size.s);	

		ret = fimc_v4l2_qbuf(m_cam_fd,index);
	}



	stopPreview();
	DestroyCamera();
	fb_err:
	close(fb_fd);

	exit(1);
}
