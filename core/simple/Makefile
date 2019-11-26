VERSION := `uname -r`
KDIR := /lib/modules/$(VERSION)/build
WD := `pwd`
SRC := $(WD)/src
BLOB := $(SRC)/blob.c

kbuild:
	cd "$(SRC)"
	if [ -e "$(BLOB)" ]; then rm "$(BLOB)"; fi
	cat "$(SRC)"/*.c > "$(BLOB)"
	make -C "$(KDIR)" M="$(SRC)" modules

clean:
	cd "$(SRC)"
	rm *.ko modules.order Module.symvers *.o
	make -C "$(KDIR)" M="$(SRC)" clean 

