# ROOTKIT 
A basic rootkit Linux kernel module for exploiting kernel functions and user data. 

> This rootkit was developed and intended for Ubuntu 16.04 xenial on
> kernel version x86_64 4.15.0-66-generic

### Rootkit functionalities:
 - File hiding
 - Process hiding
 - Process privilege escalation
 - User hiding
 - Hooking custom system calls 
## Prompt/Introduction

After attackers manage to gain access to a remote (or local) machine and elevate their privileges to "root", they typically want to maintain their access, while hiding their presence from the normal users and administrators of the system.

In this project, you are asked to design and implement a basic rootkit for the Linux operating system (you can choose the exact distribution and kernel number). This rootkit should have the form of a loadable kernel module which when loaded into the kernel (by the attacker with root privileges) will do the following:

    -Hide specific files and directories from showing up when a user does "ls" and similar commands (you have to come up with a protocol that allows attackers to change these)

    -Modify the /etc/passwd and /etc/shadow file to add a backdoor account while returning the original contents of the files (pre-attack) when a normal user requests to see the file

    -Hides specific processes from the process table when a user does a "ps"

    -Give the ability to a malicious process to elevate its uid to 0 (root) upon demand (again this involves coming up with a protocol for doing that)

Note that all of these should happen by intercepting the appropriate system calls in the Linux kernel and modifying the results. You should not perform the above by replacing the system binaries (like "ls", or "ps").

## Installation
This module is composed of two components, the core and the driver. Each component will have to built sepeartely. 

>The following commands assume that this project folder is the current directory.
### Core
The core program is the Linux kernel module that will handle system call hooking and other operations in the kernel space. All of these commands should be ran in the *core* directory. 
>**You must have root privileges to run these commands**.

    $ cd core

#### Build

To compile the module, run `make`

    $ make

#### Install

To install the module, run `insmod`

    $ insmod src/blob.ko  

#### Build & Install
 This one-liner will build & install the module for us, provided by our Makefile.

    $ make install

> **Note**: Our module loads with the name `blob` when displayed from `lsmod`.
> 
#### Uninstall 
To uninstall the module, run `rmmod`

    $ rmmod blob

#### Clean
In the event that you need to re-compile our module, you can clean the compiled object and kernel-object files with this command.

    $ make clean

### Driver
The driver program is used to communicate to the core program from user space. This will interface the client and the core by accepting a series of command arguements. All of these commands should be ran in the *driver* directory. 

    $ cd driver

#### Install
Install the driver program using the C compiler.

    $ cc syscall.c -o syscall

#### Uninstall
To uninstall the driver program, delete it.

    $ rm syscall

## Usage

Client makes a system call, with these arguments:

    syscall(secret system call number, secret, command, ...)

Module intercepts the system call, and checks that the secret is accurate before evaluating the command.

Use the `driver` script to send remote commands to the module.  

    $ ./driver COMMAND [ULONG | 0xHEXULONG | CARRAY ...]

|Command| Description |
|--|--|
| elevate PID | set a process's UID to root|
| drop PID| set a process's UID from root to its original |
| fugitive UID| remove a user from "/etc/passwd" and "/etc/shadow" |
| unfugitive UID | replace a user from "/etc/passwd" and "/etc/shadow" |
| hide PATH| hide an entity ("/proc" paths are treated as processes) |
| show PATH| show a hidden entity ("/proc" paths are treated as processes) |
## File Hiding
## Process Hiding
## Process privilege escalation
## User hiding
## Hooking custom system calls 
### Extending the Base Code
The base code is meant to be extended, in fact:
it barely does anything on its own.
To run your own code with the module,
there are 4 hooks (which are called in the following order):
1. `SCTM_INIT_PRE_HOOK`
2. `SCTM_INIT_POST_HOOK`
3. `SCTM_EXIT_PRE_HOOK`
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

## License
Copyright 2019. By collabrators
[Bailey Defino](https://bdefino.github.io), [Bryan Valarezo](https://bvalarezo.github.io), [Jeffery Wong](https://jeffrewong.github.io), [Jumming Liu](https://junmingl.github.io)

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