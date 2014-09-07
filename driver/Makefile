all: drivers 
obj-m += adk-aoa-skeleton.o

KSRC ?= /lib/modules/$(shell uname -r)/build   #you may change as per your kernel build
#ARM_KSRC=<ksrc-of-cross-built-kernel>
#ARCH=arm
#CC_PREFIX=<prefix-for-cross-compilation>

drivers:
	make -C $(KSRC) M=$(PWD) modules
	#make -C ARCH=$(ARCH) CROSS_COMPILE=$(CC_PREFIX) -C $(ARM_KSRC) M=$(PWD) modules
clean:
	make -C $(KSRC) M=$(PWD) clean
