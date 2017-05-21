#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#define CLED_ADDRESS_RED (0x06000000 + 0x9000)
#define CLED_ADDRESS_GREEN (0x06000000 + 0x9002)
#define CLED_ADDRESS_BLUE (0x06000000 + 0x9004)

#define ADDRESS_MAP_SIZE 0x1000
volatile unsigned short *cled_base;

MODULE_DESCRIPTION("System programming practice 1, dip switch driver");
MODULE_AUTHOR("LEE EUNU");
MODULE_LICENSE("GPL");


int cled_open(struct inode *inode, struct file *pfile)
{
	if(check_mem_region(CLED_ADDRESS, ADDRESS_MAP_SIZE)){
		printk("cled: memory already in use\n");
		return -EBUSY;
	}
	if(request_mem_region(CLED_ADDRESS, ADDRESS_MAP_SIZE, "DIPSW")==NULL){
		printk("cled:  fail to allocatie mem region\n");
		return -EBUSY;
	}
	
	cled_base = ioremap(CLED_ADDRESS, ADDRESS_MAP_SIZE);
	if(cled_base == NULL){
		printk("cled: fail to io mapping\n");
		release_mem_region(CLED_ADDRESS, ADDRESS_MAP_SIZE);
		return -ENOMEM;
	}

	return 0;
}

int pow(int x, int r) {
	if(r == 1) return x;
	else if(r == 0) return 1;
	return pow(x, r-1)*x;
}

int cled_read(struct file *pfile, char *buf, size_t count, loff_t *filePos)
{
	unsigned short rdata;
	unsigned int ret;
	unsigned int row, col;

	int i;
	for(i=0; i<4; i++) {
		unsigned short colnum = 16*pow(2,i);
		writew(colnum, cled_base);
		col = i;
		rdata = readw(cled_base);
	
		rdata &= 0x0F;		
	
		switch(rdata) {
			case 0x1:
				row = 0;
				rdata = row*4+col+1;
				ret = copy_to_user(buf, &rdata, 1);
				return 1;
				break;
			case 0x2:
				row = 1;
				rdata = row*4+col+1;
				ret = copy_to_user(buf, &rdata, 1);
				return 1;
				break;
			case 0x4:
				row = 2;
				rdata = row*4+col+1;
				ret = copy_to_user(buf, &rdata, 1);
				return 1;
				break;
			case 0x8:
				row = 3;
				rdata = row*4+col+1;
				ret = copy_to_user(buf, &rdata, 1);
				return 1;
				break;
			default:
				rdata = 0;
				ret = copy_to_user(buf, &rdata, 1);
				break;
		}
	}	
	

	return -1;
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
	.read = cled_read,
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




