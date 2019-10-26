VERSION := `uname -r`
KDIR := /lib/modules/$(VERSION)/build
WD := `pwd`

kbuild:
	make -C "$(KDIR)" M="$(WD)"

clean:
	make -C "$(KDIR)" M="$(WD)" clean

