

obj-m := oleddrv.o 
KDIR := /root/kernel
PWD := $(shell pwd)
 
export CROSS_COMPILE=arm-none-linux-gnueabi-
all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
        
clean:
	-rm *.o *.ko *.mod.c .*.cmd modules.order Module.symvers 
