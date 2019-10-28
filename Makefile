obj-m += skel.o
all:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test:
		sudo dmesg -C
		sudo insmod rk.ko
		sudo rmmod rk.ko
		dmesg