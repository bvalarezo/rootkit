K_dir = /lib/modules/$(shell uname -r)/build
CWD = $(shell pwd)

obj-m += evil-account.o

all:
	make -C $(K_dir) M=$(CWD) modules

clean:
	make -C $(K_dir) M=$(CWD) clean
