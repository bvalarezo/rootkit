# README
## License
Copyright 2019 Bailey Defino
<https://bdefino.github.io>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

## Extending the Base Code
The base code is meant to be extended, in fact:
it barely does anything on its own.
To run your own code with the module,
there are 4 hooks (which are called in the following order):
1. `SCTM_INIT_PRE_HOOK`
2. `SCTM_INIT_POST_HOOK`
3. `SCTM_EXIT_PRE_HOOK`
and
4. `SCTM_EXIT_POST_HOOK`

The first 2 are called after the module is loaded,
and the other 2 are called before the module is unloaded.

### Hooking the System Call Table with `sctm`
Using `sctm`, this boils down to 3 basic steps:
1. represent your system call handler as `struct sctm_hook`
2. write an initialization function to call `sctm_hook` with your handler
and
3. define `SCTM_INIT_POST_HOOK` to a function or function-like macro containing the hooking code.

See "./src/test.c" for a full example.

### Building `sctm`
A few quirks of the `Kbuild` process make this a bit finnicky:
declarations/definitions must occur in a very specific context and order.
The easiest solution is to include necessary definitions ASAP,
include all source files in a blob, and compile the blob.
If defining any of the `SCTM_*_HOOK` functions,
the following must be done:
1. definitions (including for the initialization hook function)
  MUST be in a separate header file
and
2. `SCTM_INCLUDE` MUST be defined at compile time
  (this can be done by modifying the `INCLUDE_FIRST` definition in "./src/Kbuild")

See "./include/test.h", "./src/test.Kbuild", and "test.Makefile" for full examples.

