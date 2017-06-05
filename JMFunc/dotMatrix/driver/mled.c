#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#define MLED_ADDRESS (0x06000000 + 0x1000)
#define ADDRESS_MAP_SIZE 0x1000
volatile unsigned short *mled_base;

MODULE_DESCRIPTION("System programming practice 1, mled driver");
MODULE_AUTHOR("Jongmin Park");
MODULE_LICENSE("GPL");

unsigned char unconverted[12][35] = {
	{	// 0
		0, 1, 1, 1, 0,
		1, 0, 0, 0, 1,
		1, 1, 0, 0, 1,
		1, 0, 1, 0, 1,
		1, 0, 0, 1, 1,
		1, 0, 0, 0, 1,
		0, 1, 1, 1, 0 
	},
	{	// 1
		0, 0, 1, 0, 0,
		0, 1, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 1, 1, 1, 0
	},
	{	// 2
		1, 1, 1, 1, 0,
		0, 0, 0, 0, 1,
		0, 0, 0, 1, 0,
		0, 0, 1, 0, 0,
		0, 1, 1, 0, 0,
		1, 0, 0, 0, 0,
		0, 1, 1, 1, 1
	},
	{	// 3 
		1, 1, 1, 1, 0,
		0, 0, 0, 0, 1,
		0, 0, 0, 0, 1,
		0, 1, 1, 1, 0,
		0, 0, 0, 0, 1,
		0, 0, 0, 0, 1,
		1, 1, 1, 1, 0
	},
	{	// 4 
		0, 0, 1, 0, 0,
		0, 1, 1, 0, 0,
		1, 0, 1, 0, 0,
		1, 0, 1, 0, 0,
		1, 1, 1, 1, 1,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0
	},
	{	// 5 
		0, 1, 1, 1, 1,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		0, 0, 0, 0, 1,
		0, 0, 0, 0, 1,
		1, 1, 1, 1, 0
	},
	{	// 6 
		0, 1, 1, 1, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 1, 1, 1, 0,
		1, 0, 0, 0, 1,
		1, 0, 0, 0, 1,
		0, 1, 1, 1, 0
	},
	{	// 7 
		1, 1, 1, 1, 1,
		0, 0, 0, 0, 1,
		0, 0, 0, 0, 1,
		0, 0, 1, 1, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0
	},
	{	// 8 
		0, 1, 1, 1, 0,
		1, 0, 0, 0, 1,
		1, 0, 0, 0, 1,
		0, 1, 1, 1, 0,
		1, 0, 0, 0, 1,
		1, 0, 0, 0, 1,
		0, 1, 1, 1, 0
	},
	{	// 9
		0, 1, 1, 1, 0,
		1, 0, 0, 0, 1,
		1, 0, 0, 0, 1,
		0, 1, 1, 1, 1,
		0, 0, 0, 0, 1,
		0, 0, 0, 0, 1,
		1, 1, 1, 1, 0
	},
	{	// left play 
		1, 0, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 0,
		1, 1, 0, 0, 0,
		1, 0, 0, 0, 0
	},
	{	// right play 
		1, 0, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 0,
		1, 1, 0, 0, 0,
		1, 0, 0, 0, 0
	}
};
unsigned char Font[12][7];

unsigned char pow(unsigned char n, unsigned char r) {
	if(r == 0)
		return 1;
	else if(r == 1)
		return n;
	else return n*pow(n, r-1);		
}


int mled_open(struct inode *inode, struct file *pfile)
{
	if(check_mem_region(MLED_ADDRESS, ADDRESS_MAP_SIZE)){
		printk("mled: memory already in use\n");
		return -EBUSY;
	}
	if(request_mem_region(MLED_ADDRESS, ADDRESS_MAP_SIZE, "mled")==NULL){
		printk("mled:  fail to allocatie mem region\n");
		return -EBUSY;
	}
	
	mled_base = ioremap(MLED_ADDRESS, ADDRESS_MAP_SIZE);
	if(mled_base == NULL) {
		printk("mled: fail to io mapping\n");
		release_mem_region(MLED_ADDRESS, ADDRESS_MAP_SIZE);
		return -ENOMEM;
	}

	int i, j, k;
	for(i=0; i<12; i++) {
		unsigned char psum;

		for(j=0; j<7; j++) {
			psum = 0;

			for(k=0; k<5; k++)
				if(unconverted[i][j*5+k] == 1)
					psum += pow(2, k);
		
			Font[i][j] = psum;
	//		printk("%X ",psum);
		}
	//	printk("\n\n",psum);
	}
	
	return 0;
}


ssize_t mled_write (struct file *pfile, const char *buf ,size_t count, loff_t *filePos)
{
	unsigned short wdata;
	unsigned int ret;

	ret = copy_from_user(&wdata, buf, 4);	

	int i;
	volatile char swdata[3];
	snprintf(swdata, 3, "%d", wdata);	
		
	writew(0xFF00, mled_base);				
	writew(0xFF00, (mled_base+1));
	
	unsigned short res;
	for(i=0; i<2; i++) {

		if(swdata[i]) {
			unsigned char num;
			unsigned char * targetFont;
			int j;

			if(wdata == 100 && i == 0) // display left play 
				targetFont = Font[10];
			else if(wdata == 100 && i == 1) // display right play 
				targetFont = Font[11];
			else { 
				num = swdata[i] - '0';
				targetFont = Font[num];
			}

			for(j=6; j>=0; j--) { // 7 row
				res = 1 << j;
				res |= (~targetFont[j]) << 8; // col

				if(i==0 && strlen(swdata) != 1)
					writew(res, mled_base);
				else				
					writew(res, (mled_base+1));	
			}		
		} // end of if
	}

	return 1;
}
int mled_release(struct inode *indoe, struct file *pfile)
{
	if(mled_base != NULL){
		iounmap(mled_base);
		mled_base = NULL;
		release_mem_region(MLED_ADDRESS, ADDRESS_MAP_SIZE);
	}
	return 0;
}

struct file_operations mled_fops = {
	.owner = THIS_MODULE,
	.open = mled_open,
	.write = mled_write,
	.release = mled_release
};
struct miscdevice mled_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mled",
	.fops = &mled_fops,
};



static int __init mled_init(void)
{
	int res;
	res = misc_register(&mled_device);
	if(res){
		printk("fail to register the device\n");
		return res;
	}
	return 0;
}

static void __exit mled_exit(void)
{
	misc_deregister(&mled_device);
}

module_init(mled_init)
module_exit(mled_exit)




