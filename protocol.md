# protocol
## overview
1. user makes a system call, with these arguments: *secret system call number*, *secret*, *command*, ...
2. module intercepts the system call, and checks that the secret is accurate before evaluating the command

## commands
- `command ARGS` -> execute the arguments as root
- `descend PID` -> set a process's EUID to its UID
- `elevate PID` -> set a process's EUID to root
- `fugitive UID` -> remove a user's ID 
- `hide PATH` -> hide an entity ("/proc" paths are treated as processes)
- `show PATH` -> show a hidden entity ("/proc" paths are treated as processes)

