CC := gcc
DEFS := -D__KERNEL__ -DLINUX -DMODULE
INCLUDE := -isystem /usr/src/kernel-headers-$(KERNEL)/include
KERNEL := `uname -r`
WARN := -Wall -Werror -Wmissing-prototypes -Wstrict-prototypes
CFLAGS := $(DEFS) $(INCLUDE) $(WARN)
OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

all: $(OBJS)

.PHONY: clean

clean:
	rm -f *.o

