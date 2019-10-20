BUILD_DIR := /lib/modules/`uname -r`/build
objs += sctm.o

all:
	make -C "$(BUILD_DIR)" M="$$PWD" modules

.PHONY: clean

clean:
	make -C "$(BUILD_DIR)" M="$$PWD" clean
