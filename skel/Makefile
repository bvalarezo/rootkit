CC := gcc
DEFS := -D__KERNEL__ -DLINUX -DMODULE
KERNEL := `uname -r`
INCLUDE := -I include -isystem /usr/src/kernel-headers-$(KERNEL)/include
WARN := -Wall -Werror -Wmissing-prototypes -Wstrict-prototypes
CFLAGS := -c $(DEFS) $(INCLUDE) $(WARN)
SRCS := $(wildcard *.c)

all: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)

.PHONY: clean

clean:
	rm -f *.o

