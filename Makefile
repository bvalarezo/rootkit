Obj-m := rootkit.o
KERNEL_DIR = /lib/modules/$(shell uname -r)/build
#KERNEL_DIR = ~/buildroot/buildroot-2019.02.6/output/build/linux-4.15/
#KERNEL_DIR = ~/
PWD = $(shell PWD)
all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD)
clean:
	rm -rf *.o *.ko *.symvers *.mod.* *.order