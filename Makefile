CC := gcc
DEFS := -D__KERNEL__ -DLINUX -DMODULE
iKERNEL := `uname -r`
INCLUDE := -I include -isystem /usr/src/kernel-headers-$(KERNEL)/include
WARN := -Wall -Werror -Wmissing-prototypes -Wstrict-prototypes
CFLAGS := -c $(DEFS) $(INCLUDE) $(WARN)

all:
	$(CC) $(CFLAGS) `find | grep ".c$$"`

.PHONY: clean

clean:
	rm -f *.o

