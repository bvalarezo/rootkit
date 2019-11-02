.PHONY: all
obj-m := rootkit.o
KERNEL_DIR = /lib/modules/$(shell uname -r)/build
PWD = $(shell pwd)
all: rootkit
rootkit:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD)

clean:
	rm -rf *.o *.ko *.symvers *.mod.* *.order
