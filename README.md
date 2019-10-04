# rootkit
CSE331 Project 1 

Project 1: Linux Rootkit

After attackers manage to gain access to a remote (or local) machine and elevate their privileges to "root", they typically want to maintain their access, while hiding their presence from the normal users and administrators of the system.

In this project, you are asked to design and implement a basic rootkit for the Linux operating system (you can choose the exact distribution and kernel number). This rootkit should have the form of a loadable kernel module which when loaded into the kernel (by the attacker with root privileges) will do the following:

    -Hide specific files and directories from showing up when a user does "ls" and similar commands (you have to come up with a protocol that allows attackers to change these)

    -Modify the /etc/passwd and /etc/shadow file to add a backdoor account while returning the original contents of the files (pre-attack) when a normal user requests to see the file

    -Hides specific processes from the process table when a user does a "ps"

    -Give the ability to a malicious process to elevate its uid to 0 (root) upon demand (again this involves coming up with a protocol for doing that)

Note that all of these should happen by intercepting the appropriate system calls in the Linux kernel and modifying the results. You should not perform the above by replacing the system binaries (like "ls", or "ps").

=======================================================================
We have to hijack the System Calls. We want to modify the SysCall Table to point to our malicious functions. 
The backdoor account should not be affected by any of these modiications.

First, we need to find the SysCall table 

Find the index of the SysCalls that we want to replace to perform the above goals.

Create a functon that replaces these SysCalls while holding the pointer to the original SysCall function.

Protocol to communicate to the module:
    - Create a new SysCall (SysCall index n in the SysCall table) with the module installation which will point to a function that we define that will handle communication to the module.
    - Only the backdoor account should be able to successully execute the new SysCall.
    
For first time installation:
    - We are assumed to have root access.
    - Rootkit will create a backdoor account on installation, switch to the new backdoor account (via "su backdooraccount"), and print out "Syscall N".
 
Subsequent accesses to the compromised system:
    - Log in via through the backdoor account.
    
SysCall N:
    - Potentially prompt attacker "What do you want to do?"
        - Hide speciic files
        - Hide processes from the process table
        - Control privilege execution for processes
        - Delete itself on uninstallation
    - Lists all of the files and processes already hidden
        - Directory that contains two directories
            - One for processes and one for filepaths

======================================================================================================================
Misc. Specifications:
- Kernel Version Choice: Doesn't matter
- Potentially add a new module to handle the direct calling of syscalls.
