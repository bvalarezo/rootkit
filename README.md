# Hello World(Documentation of the module)

How to install the module:
    - sudo insmod rootkit.ko (If you are using kernel 4.15.0-45-generic (default on ubuntu 16.04); otherwise do make clean all to recompile the module)
How to remove the module:
    - sudo rmmod rootkit
    
How to use the hide files from ls/tree part of the module:
- Name your file with the prefix @evil@ and it will be hidden.

How to use the hide processes part of the module with the driver:

- Assuming the driver was compiled with the name syscall:
    - ./syscall 82 @@evil@(processname) @@evil@(action)
        - Action can be one of the following:
            - addEntry 
                - Will return 0 if successful or -ENOMEM if the arraylist could not resize due to full array or memory for the arraylist could not be allocated at the initialization phase of the module
            - removeEntry
                - Will return 0 on successful removal or if the entry does not exist.
- The module will also hide by default any processes that start with the prefix @evil@.

Examples:
- If we want to hide bash from ps:
    - ./syscall 82 @@evil@bash @@evil@addEntry
- If we want to show bash in ps:
    - ./syscall 82 @@evil@bash @@evil@removeEntry
- If we want to hide and create process called weewoo from ps without using the driver:
    - nano @evil@weewoo.sh
    - To make this visible again just rename the file and remove the @evil@ prefix.
- If we want to create and hide a file called somefilename from ls/tree:
    - nano @evil@somefilename
    
Implementation details of process hiding:
- The module hooks the syscall rename which is 82 in Ubuntu 16.04. Although this shouldn't change across systems, you may need to find the system call number of rename which can be found in dmesg or kern.log.   

- If the driver passes in null values for the strings or the strings do not contain the prefix, it will default to the original functionality of rename.
    - Because of this, the filenames @evil@addEntry and @evil@removeEntry are not recommended to be used and will not be able to be renamed.

- Whenever a user uses ps or top/htop, the syscall getdents is used to obtain a list of files/directories in /proc.
    - Directories in /proc are named an integer number which corresponds to the pid of the process. 

- The getdents syscall hook will check if the current process is calling getdents on /proc and then will proceed to do the following:
    - Get the name of the directory listing in the structure of directories.
        - The structure of directories essentially functions as a linked list where we have to use offsets instead of pointers to next and back.
    - Obtain process information based on integer obtained from the directory name.
    - Check if the process name is in the arraylist of processes to hide or if the processname has the prefix @evil@.
        - It's important to note that process names are limited to 15 characters and thus the comparison will only check the first 15 characters.
    - Remove the directory entry by shifting the entries down and change the number of bytes read in the return value.

Implementation details of file hiding:
- It essentially works the same as process hiding except it checks if the directory name contains the @evil@ prefix and then does the same shiting of directory entries down etc.
- No other processing of the directory name is needed.
