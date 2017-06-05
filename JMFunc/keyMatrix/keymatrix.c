#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#define KEY_MATRIX_ADDRESS (0x06000000 + 0x4000)
#define ADDRESS_MAP_SIZE 0x1000
volatile unsigned short *keymatrix_base;

MODULE_DESCRIPTION("System programming practice 1, dip switch driver");
MODULE_AUTHOR("LEE EUNU");
MODULE_LICENSE("GPL");


int keymatrix_open(struct inode *inode, struct file *pfile)
{
	if(check_mem_region(KEY_MATRIX_ADDRESS, ADDRESS_MAP_SIZE)){
		printk("keymatrix: memory already in use\n");
		return -EBUSY;
	}
	if(request_mem_region(KEY_MATRIX_ADDRESS, ADDRESS_MAP_SIZE, "DIPSW")==NULL){
		printk("keymatrix:  fail to allocatie mem region\n");
		return -EBUSY;
	}
	
	keymatrix_base = ioremap(KEY_MATRIX_ADDRESS, ADDRESS_MAP_SIZE);
	if(keymatrix_base == NULL){
		printk("keymatrix: fail to io mapping\n");
		release_mem_region(KEY_MATRIX_ADDRESS, ADDRESS_MAP_SIZE);
		return -ENOMEM;
	}

	return 0;
}

int pow(int x, int r) {
	if(r == 1) return x;
	else if(r == 0) return 1;
	return pow(x, r-1)*x;
}

int keymatrix_read(struct file *pfile, char *buf, size_t count, loff_t *filePos)
{
	unsigned short rdata;
	unsigned int ret;
	unsigned int row, col;

	int i;
	for(i=0; i<4; i++) {
		unsigned short colnum = 16*pow(2,i);
		writew(colnum, keymatrix_base);
		col = i;
		rdata = readw(keymatrix_base);
	
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

int keymatrix_release(struct inode *indoe, struct file *pfile)
{
	if(keymatrix_base != NULL){
		iounmap(keymatrix_base);
		keymatrix_base = NULL;
		release_mem_region(KEY_MATRIX_ADDRESS, ADDRESS_MAP_SIZE);
	}
	return 0;
}

struct file_operations keymatrix_fops = {
	.owner = THIS_MODULE,
	.open = keymatrix_open,
	.read = keymatrix_read,
	.release = keymatrix_release
};
struct miscdevice keymatrix_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "keymatrix",
	.fops = &keymatrix_fops,
};



static int __init keymatrix_init(void)
{
	int res;
	res = misc_register(&keymatrix_device);
	if(res){
		printk("fail to register the device\n");
		return res;
	}
	return 0;
}

static void __exit keymatrix_exit(void)
{
	misc_deregister(&keymatrix_device);
}

module_init(keymatrix_init)
module_exit(keymatrix_exit)




