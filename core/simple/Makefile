BLOB := blob.c
VERSION := `uname -r`
KDIR := /lib/modules/$(VERSION)/build
WD := `pwd`
SRC := $(WD)/src
SRCS := sctm.c test.c

kbuild:
	#if [ -e "$(BLOB)" ]; then rm "$(BLOB)"; fi
	#cd "$(SRC)" && cat $(SRCS) > "$(BLOB)"
	make -C "$(KDIR)" M="$(SRC)" modules

clean:
	cd "$(SRC)" && rm .cache.mk *.ko modules.order Module.symvers *.o
	make -C "$(KDIR)" M="$(SRC)" clean 

