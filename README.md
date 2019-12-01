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
>**You must have root privileges to run these installation commands**.

    cd core

#### Build

To compile the module, run `make`

    make

#### Install

To install the module, run `insmod`

    insmod src/blob.ko  

#### Build & Install
 This one-liner will build & install the module for us, provided by our Makefile.

    make install

> **Note**: Our module loads with the name `blob` when displayed from `lsmod`.
> 
#### Uninstall 
To uninstall the module, run `rmmod`

    rmmod blob

#### Clean
In the event that you need to re-compile our module, you can clean the compiled object and kernel-object files with this command.

    make clean

### Driver
The driver program is used to communicate to the core program from user space. This will interface the client and the core by accepting a series of command arguements. All of these commands should be ran in the *driver* directory. 

    cd driver

#### Install
Install the driver program using the C compiler.

    cc syscall.c -o syscall

#### Uninstall
To uninstall the driver program, delete it.

    rm syscall

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
Commands such as ps or tree make use of the getdents(GETDirectoryENTrieS) syscall to get a list of directory entries at the given directory.

The getdents syscall creates a linked list of directory entry structures (defined by linux_dirent and linux_dirent64) and fills it into a caller provided pointer and then returns the number of bytes written into the pointer back to the caller.

In order to hide files, we just call the original getdents syscall to generate the structure and then we need to remove the desired linux_dirent structures from the linked list and alter the count of bytes returned by getdents.

To determine what counts as a desired linux_dirent structure to remove, we check if the d_name field in the linux_dirent structure contains a set prefix in the file name. 
If it contains said prefix, we will remove it from the linked list by shifting the entries ahead of the one we want to delete onto the current one.

### Examples
To hide a file, you must add the prefix to the front of its name (3v!1 is the prefix for our rootkit).
Creating a hidden text file called helloworld.txt using the nano text editor (you may use any text editor of your choice):

    $ nano 3v!1helloworld.txt
    
Hiding an existing text file called helloworld.txt:

    $ mv helloworld.txt 3v!1helloworld.txt
    
To unhide a file, you must rename the file to remove the prefix.
Unhiding a file called helloworld.txt which already has the specified hiding prefix:
    
    $ mv 3v!1helloworld.txt helloworld.txt

NOTE: Make sure you keep track of the paths of directories/files that are hidden as they will be hidden to you as well.

## Process Hiding
Hiding a process works in a similar manner to file hiding.
Commands such as ps, top, htop etc. makes use of the getdents syscall on the /proc directory to obtain details of the current processes running.
The /proc directory is comprised of files and directories that contain details about the system such as resource usage.
The /proc directory also contains directories which are named with an integer corresponding the the PID of a process which is what we will use for hiding processes.

To hide processes, we do the same process as hiding a file except in order to determine what linux_dirent structure to remove, we have to perform some extra steps.
    - Since the d_name field of the linux_dirent only gives us the PID of the process which the linux_dirent belongs to, we have to perform a lookup of the pid to get its task_struct.
    - The struct task_struct has a comm field which contains the command name of the process which we then check if it contains the prefix.
    - If it does contain the prefix then we remove the linux_dirent structure, otherwise we check if the command name is in our arraylist of processes to hide. If the process name is in our arraylist, we remove the linux_dirent structure, otherwise it will be shown.
        - The arraylist is an array of strings that is allocated on the installation of the module and is resized when the maximum capacity is reached.

### Examples
Process hiding can be done in two ways: Naming the executable file with the prefix 3v!1 or by using the driver to add a specific process name.
Creating a shell script called helloworld.sh that will be hidden using nano text editor (you may use whatever text editor you want):

    $ nano 3v!1helloworld.sh
    
Hiding a shell script called helloworld.sh from the process list:

    $ mv helloworld.sh 3v!1helloworld.sh
    
Showing a shell script called helloworld.sh from the process list on next execution:

    $ mv 3v!1helloworld.sh helloworld.sh
 
NOTE: If the process spawns subprocesses, those subprocesses will NOT be hidden (e.g. if helloworld.sh uses sleep 30, sleep will show up in ps). You must use the driver to hide these subprocesses which is shown in the next examples.

Hiding all processes named bash using the driver:

    $ ./driver (to be added later)
    
Showing all previously hidden processes named bash using the driver:

    $ ./driver (to be added later)

## Process privilege escalation
In Linux, a process structure is defined by the task_struct.

Escalating processes is done by modifying a task_struct's credentials. This can be accomplished by overwriting the credentials struct within the task_struct.

All of a task’s credentials are held in (uid, gid) or through (groups, keys, LSM security) a refcounted structure of type ‘struct cred’. Each task points to its credentials by a pointer called ‘cred’ in its task_struct.

A credential's struct can be accessed within the task_struct

    pcred = (struct cred *)task->cred;

In order to escalate, we can set the credentials equal to root.

    pcred->uid.val = 0;
    pcred->suid.val = 0;
    pcred->euid.val = 0;
    pcred->fsuid.val = 0;
    pcred->gid.val = 0;
    pcred->sgid.val = 0;
    pcred->egid.val = 0;
    pcred->fsgid.val = 0;

Our rootkit will also save the process's original credentials in a custom pid_node struct.

    typedef struct pid_node {
        pid_t 		pid;
        kuid_t uid, suid, euid, fsuid;
        kgid_t gid, sgid, egid, fsgid;
        struct pid_node *prev, *next;
    } PID_NODE;

And back it up in case the user wants to drop the privileges.

    new_node->uid = pcred->uid;
    new_node->suid = pcred->suid;
    new_node->euid = pcred->euid;
    new_node->fsuid = pcred->fsuid;
    new_node->gid = pcred->gid;
    new_node->sgid = pcred->sgid;
    new_node->egid = pcred->egid;
    new_node->fsgid = pcred->fsgid;

### Example
Escalating a process with a PID 12345

    ./driver elevate 12345
Dropping the escalated process back to its original UID

    ./driver drop 12345

>**Note:** You may only drop processes you have escalated before.

## Backdoor Account
When the rootkit is first loaded, it adds backdoor account into /etc/passwd and /etc/shadow if the account doesn't exist already. It also hides backdoor account while the rootkit is loaded by hijacking write syscall and change the output buffer.

### Backdoor account info
> Username = fugitive

> Password = Password123

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
## Sources
Helpful resources used during the development of this project

1. https://blog.trailofbits.com/2019/01/17/how-to-write-a-rootkit-without-really-trying/
2. https://davejingtian.org/2019/02/25/syscall-hijacking-in-2019/
3. https://www.kernel.org/doc/Documentation/security/credentials.txt
4. https://github.com/torvalds/linux/blob/master/include/linux/sched.h
5. http://tldp.org/LDP/lkmpg/2.6/html/lkmpg.html
6. https://www.kernel.org/doc/html/latest/media/uapi/v4l/io.html
7. https://mammon.github.io/Text/kernel_read.txt
8. https://stackoverflow.com/questions/1184274/read-write-files-within-a-linux-kernel-module
9. https://elixir.bootlin.com/
10. http://man7.org/linux/man-pages/man2/getdents.2.html
11. https://stackoverflow.com/questions/2103315/linux-kernel-system-call-hooking-example
12. https://docs.huihoo.com/doxygen/linux/kernel/3.7/structdentry.html
13. http://tuxthink.blogspot.com/2012/07/module-to-find-task-from-its-pid.html
14. https://ccsl.carleton.ca/~dmccarney/COMP4108/a2.html
15. https://stackoverflow.com/questions/8250078/how-can-i-get-a-filename-from-a-file-descriptor-inside-a-kernel-module

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
