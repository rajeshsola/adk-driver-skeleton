all: drivers 
DRIVER = adk-aoa-skeleton
OBJS = adk-common.o adk-init.o aoa-skeleton.o adk-sysfs.o

obj-m += $(DRIVER).o
$(DRIVER)-objs := $(OBJS)

KSRC ?= /lib/modules/$(shell uname -r)/build   #you may change as per your kernel build
drivers:
	make -C $(KSRC) M=$(PWD) modules
clean:
	make -C $(KSRC) M=$(PWD) clean
