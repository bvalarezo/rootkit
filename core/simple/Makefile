BLOB := blob.c
VERSION := `uname -r`
KDIR := /lib/modules/$(VERSION)/build
WD := `pwd`
SRC := $(WD)/src
SRCS := desired-test.c desired-sctm.c # order matters (`sctm` MUST be last)

kbuild:
	if [ -e "$(BLOB)" ]; then rm "$(BLOB)"; fi
	cd "$(SRC)" && cat $(SRCS) > "$(BLOB)"
	make -C "$(KDIR)" M="$(SRC)" modules

clean:
	cd "$(SRC)" && rm *.ko modules.order Module.symvers *.o
	make -C "$(KDIR)" M="$(SRC)" clean 

