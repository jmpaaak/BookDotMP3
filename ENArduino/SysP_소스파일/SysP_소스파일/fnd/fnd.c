#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#define FND_ADDRESS (0x06000000 + 0x6000)
#define ADDRESS_MAP_SIZE 0x1000
volatile unsigned short *FND_base;

MODULE_DESCRIPTION("System programming practice 1, FND driver");
MODULE_AUTHOR("LEE EUNU");
MODULE_LICENSE("GPL");


int FND_open(struct inode *inode, struct file *pfile)
{
	if(check_mem_region(FND_ADDRESS, ADDRESS_MAP_SIZE)){
		printk("FND: memory already in use\n");
		return -EBUSY;
	}
	if(request_mem_region(FND_ADDRESS, ADDRESS_MAP_SIZE, "FND")==NULL){
		printk("FND:  fail to allocatie mem region\n");
		return -EBUSY;
	}
	
	FND_base = ioremap(FND_ADDRESS, ADDRESS_MAP_SIZE);
	if(FND_base == NULL){
		printk("FND: fail to io mapping\n");
		release_mem_region(FND_ADDRESS, ADDRESS_MAP_SIZE);
		return -ENOMEM;
	}
	return 0;
}


ssize_t FND_write (struct file *pfile, const char *buf ,size_t count, loff_t *filePos)
{
	unsigned short wdata;
	unsigned int ret;
	ret = copy_from_user(&wdata, buf, 2);
	printk("wdata %X\n\n", wdata);
	writew(wdata,FND_base);
	return count;
}
int FND_release(struct inode *indoe, struct file *pfile)
{
	if(FND_base != NULL){
		iounmap(FND_base);
		FND_base = NULL;
		release_mem_region(FND_ADDRESS, ADDRESS_MAP_SIZE);
	}
	return 0;
}

struct file_operations FND_fops = {
	.owner = THIS_MODULE,
	.open = FND_open,
	.write = FND_write,
	.release = FND_release
};
struct miscdevice FND_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "FND",
	.fops = &FND_fops,
};



static int __init FND_init(void)
{
	int res;
	res = misc_register(&FND_device);
	if(res){
		printk("fail to register the device\n");
		return res;
	}
	return 0;
}

static void __exit FND_exit(void)
{
	misc_deregister(&FND_device);
}

module_init(FND_init)
module_exit(FND_exit)




