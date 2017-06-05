#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#define BLED_ADDRESS (0x06000000 + 0x5000)
#define ADDRESS_MAP_SIZE 0x1000
volatile unsigned short *bled_base;

MODULE_DESCRIPTION("System programming practice 1, bled driver");
MODULE_AUTHOR("LEE EUNU");
MODULE_LICENSE("GPL");

unsigned char pow(unsigned char n, unsigned char r) {
	if(r == 0)
		return 1;
	else if(r == 1)
		return n;
	else return n*pow(n, r-1);		
}


int bled_open(struct inode *inode, struct file *pfile)
{
	if(check_mem_region(BLED_ADDRESS, ADDRESS_MAP_SIZE)){
		printk("bled: memory already in use\n");
		return -EBUSY;
	}
	if(request_mem_region(BLED_ADDRESS, ADDRESS_MAP_SIZE, "bled")==NULL){
		printk("bled:  fail to allocatie mem region\n");
		return -EBUSY;
	}
	
	bled_base = ioremap(BLED_ADDRESS, ADDRESS_MAP_SIZE);
	if(bled_base == NULL) {
		printk("bled: fail to io mapping\n");
		release_mem_region(BLED_ADDRESS, ADDRESS_MAP_SIZE);
		return -ENOMEM;
	}
	
	return 0;
}

ssize_t bled_write (struct file *pfile, const char *buf ,size_t count, loff_t *filePos)
{
	unsigned char wdata;
	unsigned char ret;

	copy_from_user(&wdata, buf, 1);	
	//printk("wdata %X\n\n", wdata);	
	switch(wdata) {
		case 1:
			wdata = 0x01;
			break;
		case 2:
			wdata = 0x03;
			break;
		case 3:
			wdata = 0x07;
			break;
		case 4:
			wdata = 0x0F;
			break;
		case 5:
			wdata = 0x1F;
			break;
		case 6:
			wdata = 0x3F;
			break;
		case 7:
			wdata = 0x7F;
			break;
		case 8:
			wdata = 0xFF;
			break;
	}
	writeb(wdata, bled_base);

	return 1;

}
int bled_release(struct inode *indoe, struct file *pfile)
{
	if(bled_base != NULL){
		iounmap(bled_base);
		bled_base = NULL;
		release_mem_region(BLED_ADDRESS, ADDRESS_MAP_SIZE);
	}
	return 0;
}

struct file_operations bled_fops = {
	.owner = THIS_MODULE,
	.open = bled_open,
	.write = bled_write,
	.release = bled_release
};
struct miscdevice bled_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "bled",
	.fops = &bled_fops,
};



static int __init bled_init(void)
{
	int res;
	res = misc_register(&bled_device);
	if(res){
		printk("fail to register the device\n");
		return res;
	}
	return 0;
}

static void __exit bled_exit(void)
{
	misc_deregister(&bled_device);
}

module_init(bled_init)
module_exit(bled_exit)




