# Copyright 2019 Bailey Defino
# <https://bdefino.github.io>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
BLOB := blob.c
CLEAN_TARGETS := "$(BLOB)" .cache.mk *.ko modules.order Module.symvers *.o
VERSION := `uname -r`
KDIR := /lib/modules/$(VERSION)/build
WD := `pwd`
SRC := $(WD)/src
SRCS := sctm.c test.c

kbuild:
	if [ -e "$(BLOB)" ]; then rm "$(BLOB)"; fi
	cd "$(SRC)" && cat $(SRCS) > "$(BLOB)"
	make -C "$(KDIR)" M="$(SRC)" modules

clean:
	cd "$(SRC)" && for TARGET in $(CLEAN_TARGETS); do [ -e "$${TARGET}" ] && rm "$${TARGET}"; done
	make -C "$(KDIR)" M="$(SRC)" clean

install:
	make && sudo insmod "$(SRC)"/*.ko

