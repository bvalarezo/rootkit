#!/bin/sh
# driver script for `sctm`'s internals
# requires `./syscall`
CALL=0x66 # ideally x86-64 sys_getuid (system call 102) (getgid is implemented everywhere, and takes no arguments)
COMMAND=0x0 # numeric command code
SECRET=`echo "BB: It's Barnacle MAN.
MM: To the invisible boatmobile!" | sha256sum | cut -c-8` # compute this on the fly (not necessary)
./syscall ${CALL} ${SECRET} ${COMMAND} # hex arguments

