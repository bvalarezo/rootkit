VERSION := `uname -r`
KDIR := /lib/modules/$(VERSION)/build
WD := `pwd`

kbuild:
	make -C "$(KDIR)" M="$(WD)" modules

clean:
	rm *.ko modules.order Module.symvers *.o
	make -C "$(KDIR)" M="$(WD)" clean 

