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
INCLUDE_FIRST := test.h
EXTRA_CFLAGS = -DSCTM_INCLUDE="$(INCLUDE_FIRST)" -g "-I$(PWD)/include" -Wall -Werror -Wno-unused-function # "-Wno-unused-function" allows us to build the module sources separately
obj-m += blob.o

