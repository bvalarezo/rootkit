CC := gcc
DEFS := -D__KERNEL__ -DLINUX -DMODULE
KERNEL := `uname -r`
INCLUDE := -I include -isystem /usr/src/kernel-headers-$(KERNEL)/include
WARN := -Wall -Werror -Wmissing-prototypes -Wstrict-prototypes
CFLAGS := -c $(DEFS) $(INCLUDE) $(WARN)
SRCS := $(wildcard src/*.c) $(wildcard src/*/*.c)

all: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)

.PHONY: clean

clean:
	rm -f *.o

