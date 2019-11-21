#!/bin/sh
# (numeric) driver script for `sctm`'s internals (requires `./syscall`)
# Usage: ./$0 [0xHEXULONG ...]
CALL=0x66 # hex system call (x86-64 sys_getuid; getgid is implemented everywhere, and takes no arguments)
COMMAND=0x0 # hex command code
SECRET="0x"`echo "BB: It's Barnacle MAN.
MM: To the invisible boatmobile!" | sha256sum | cut -c-8` # secret
./syscall ${CALL} "${SECRET}" $@ # expand hex arguments

