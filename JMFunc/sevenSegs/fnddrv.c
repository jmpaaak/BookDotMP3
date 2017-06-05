#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#define FND_ADDRESS (0x06000000 + 0x6000)
#define ADDRESS_MAP_SIZE 0x1000
volatile unsigned short *fnd_base;

MODULE_DESCRIPTION("System programming practice 5, Flexible Number Display");
MODULE_AUTHOR("JONGMIN PARK");
MODULE_LICENSE("GPL");


int fnd_open(struct inode *inode, struct file *pfile)
{
	if(check_mem_region(FND_ADDRESS, ADDRESS_MAP_SIZE)){
		printk(": memory already in use\n");
		return -EBUSY;
	}
	if(request_mem_region(FND_ADDRESS, ADDRESS_MAP_SIZE, "DIPSW")==NULL){
		printk(":  fail to allocatie mem region\n");
		return -EBUSY;
	}
	
	fnd_base = ioremap(FND_ADDRESS, ADDRESS_MAP_SIZE);
	if(fnd_base == NULL){
		printk(": fail to io mapping\n");
		release_mem_region(FND_ADDRESS, ADDRESS_MAP_SIZE);
		return -ENOMEM;
	}

	return 0;
}


ssize_t fnd_write(struct file *pfile, const char *buf, size_t count, loff_t *filePos)
{
	unsigned int wdata;
	unsigned int ret;

	unsigned char numDisplay[10] = {
	    0x3F, 0x6, 0x5B, 0x4F, 0x66, 0x6D, 0x7C, 0x7, 0x7F, 0x67
	};

	unsigned char posDisplay[6] = {
	    // 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF
		0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE
	};

	copy_from_user(&wdata, buf, 4);

//	printk("user value %d\n", wdata);

	int i; // 자릿수
	int temp;
	i = 0;
	temp = wdata;
	while(temp > 9) {
	    temp /= 10;
	    i++;
	}
//	printk("index i %d\n", i);
	
	unsigned char swdata[7];
	snprintf(swdata, 7, "%d", wdata);	

//	printk("string wdata %s\n", swdata);
	unsigned char num;
	unsigned char pos;

	int numZero = 5-i;
	for(; i >= 0; i--) {
		int arrIndex = swdata[i] - '0'; 
//		printk("arr index %d\n", arrIndex);	
		pos = posDisplay[5-i-numZero];
		num = numDisplay[arrIndex];	

//		printk("num %X\n", num);
//		printk("pos %X\n\n", pos);
		volatile unsigned short res;
		res = pos << 8;
		res |= num;
		writew(res, fnd_base);
	}	
	

	return 1;
}

int fnd_release(struct inode *indoe, struct file *pfile)
{
	if(fnd_base != NULL){
		iounmap(fnd_base);
		fnd_base = NULL;
		release_mem_region(FND_ADDRESS, ADDRESS_MAP_SIZE);
	}
	return 0;
}

struct file_operations fnd_fops = {
	.owner = THIS_MODULE,
	.open = fnd_open,
	.write = fnd_write,
	.release = fnd_release
};
struct miscdevice fnd_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "fnd",
	.fops = &fnd_fops,
};



static int __init fnd_init(void)
{
	int res;
	res = misc_register(&fnd_device);
	if(res){
		printk("fail to register the device\n");
		return res;
	}
	return 0;
}

static void __exit fnd_exit(void)
{
	misc_deregister(&fnd_device);
}

module_init(fnd_init)
module_exit(fnd_exit)
