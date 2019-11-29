# Protocol
## License
Copyright 2019 Bailey Defino
<https://bdefino.github.io>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

## Overview
1. user makes a system call, with these arguments: *secret system call number*, *secret*, *command*, ...
2. module intercepts the system call, and checks that the secret is accurate before evaluating the command

## Commands
- `drop PID` -> set a process's EUID to its UID
- `elevate PID` -> set a process's EUID to root
- `ghost UID` -> remove a user from "/etc/passwd" and "/etc/shadow"
- `hide PATH` -> hide an entity ("/proc" paths are treated as processes)
- `show PATH` -> show a hidden entity ("/proc" paths are treated as processes)
- `unghost UID` -> replace a user in "/etc/passwd" and "/etc/shadow"

