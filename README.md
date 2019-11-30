
# rootkit
CSE331 Project 1 

Project 1: Linux Rootkit

# Rootkit(Documentation of the module)

How to install the module:

    $ sudo insmod rootkit.ko 

*If you are using kernel 4.15.0-45-generic (default on Ubuntu 16.04); otherwise recompile the module with*

    $ make clean all

How to remove the module:

    $ sudo rmmod rootkit

How to elevate a process's UID to 0 (root). 

    $ ./evil [elevate/drop] pid
    
|Command| Description |
|--|--|
| elevate | This command will elevate a process's UID to root|
|drop| This command will drop an elevated process's UID from root to its original UID |


