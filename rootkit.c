#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/unistd.h>
#include <asm/paravirt.h>
#include <linux/kallsyms.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/init.h>

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
asmlinkage int (*original_getdents64)(unsigned int,struct linux_dirent*, unsigned int);

asmlinkage int new_getdents(unsigned int fd,struct linux_dirent* dirp,
                    unsigned int count){
    int bytesread = original_getdents(fd,dirp,count);
    printk("succesfully executed getdents with %d bytes read \n",bytesread);
    /*int bytes;
    for(bytes = 0; bytes < bytesread;){
        linux_dirent* d = (linux_dirent*) (dirp + bytes);
        printk("%s\n",d->d_name);
        bytes += d->d_reclen;
    }*/
    printk("hello world its getdents function");
    return bytesread;
}

int init_module(){
    write_cr0 (read_cr0 () & (~ 0x10000));
    syscall_table = (unsigned long**)kallsyms_lookup_name("sys_call_table");
    printk("loaded syscall table\n");
    original_getdents = (void*)syscall_table[__NR_getdents];
	original_getdents64 = (void*)syscall_table[__NR_getdents64];
    syscall_table[__NR_getdents64] = (unsigned long*)new_getdents;
	syscall_table[__NR_getdents] = (unsigned long*)new_getdents;
    //original_getdents = *(((void**)syscall_table)+61);//[__NR_getdents64];
    ///*syscall_table[__NR_getdents64]*/ *(((void**)syscall_table)+61) = (void*)&new_getdents;
    write_cr0 (read_cr0 () | 0x10000);
    return 0;
}

void cleanup_module(){
    write_cr0 (read_cr0 () & (~ 0x10000));
    syscall_table[__NR_getdents64] = ((unsigned long*)original_getdents64);
	syscall_table[__NR_getdents] = ((unsigned long*)original_getdents);
    ///*syscall_table[__NR_getdents64]*/*(((void**)syscall_table)+61) = (void*)original_getdents;
	printk("successfully unloaded kernel module");
    write_cr0 (read_cr0 () | 0x10000);
}

MODULE_LICENSE("GPL");
