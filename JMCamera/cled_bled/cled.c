#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#define CLED_ADDRESS (0x06000000 + 0x9000)
#define ADDRESS_MAP_SIZE 0x1000
volatile unsigned short *cled_base;

MODULE_DESCRIPTION("System programming practice 7, Color RED");
MODULE_AUTHOR("JONGMIN PARK");
MODULE_LICENSE("GPL");


int cled_open(struct inode *inode, struct file *pfile)
{
	if(check_mem_region(CLED_ADDRESS, ADDRESS_MAP_SIZE)){
		printk(": memory already in use\n");
		return -EBUSY;
	}
	if(request_mem_region(CLED_ADDRESS, ADDRESS_MAP_SIZE, "DIPSW")==NULL){
		printk(":  fail to allocatie mem region\n");
		return -EBUSY;
	}
	
	cled_base = ioremap(CLED_ADDRESS, ADDRESS_MAP_SIZE);
	if(cled_base == NULL){
		printk(": fail to io mapping\n");
		release_mem_region(CLED_ADDRESS, ADDRESS_MAP_SIZE);
		return -ENOMEM;
	}

	return 0;
}


ssize_t cled_write(struct file *pfile, const char *buf, size_t count, loff_t *filePos)
{
	unsigned int wdata;
	unsigned int ret;

	copy_from_user(&wdata, buf, 4);

	// printk("user value %d\n", wdata);

	volatile unsigned short *cp_base;
	cp_base = cled_base;

	if(wdata < 13) {
	    if(wdata > 8 && wdata < 13) {
		wdata -= 8;
		cp_base += 2;
    	    } else if(wdata > 4 && wdata < 9) {
		wdata -= 4;
		cp_base += 1;
	    }

	    switch(wdata) {
		case 1:
		    writew(0, cp_base);
		    break;
		case 2:
		    writew(0x55, cp_base);
		    break;
		case 3:
		    writew(0xAA, cp_base);
		    break;
		case 4:
		    writew(0x100, cp_base);
		    break;
	    }
	}
	return 1;
}

int cled_release(struct inode *indoe, struct file *pfile)
{
	if(cled_base != NULL){
		iounmap(cled_base);
		cled_base = NULL;
		release_mem_region(CLED_ADDRESS, ADDRESS_MAP_SIZE);
	}
	return 0;
}

struct file_operations cled_fops = {
	.owner = THIS_MODULE,
	.open = cled_open,
	.write = cled_write,
	.release = cled_release
};
struct miscdevice cled_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "cled",
	.fops = &cled_fops,
};



static int __init cled_init(void)
{
	int res;
	res = misc_register(&cled_device);
	if(res){
		printk("fail to register the device\n");
		return res;
	}
	return 0;
}

static void __exit cled_exit(void)
{
	misc_deregister(&cled_device);
}

module_init(cled_init)
module_exit(cled_exit)




