## Backdoor Account
When the rootkit is first loaded, it adds backdoor account into /etc/passwd and /etc/shadow if the account doesn't exist already.
It also hides backdoor account while the rootkit is loaded by hijacking write syscall and change the output buffer.

### References:
1. http://tldp.org/LDP/lkmpg/2.6/html/lkmpg.html
2. https://davejingtian.org/2019/02/25/syscall-hijacking-in-2019/
3. https://www.kernel.org/doc/html/latest/media/uapi/v4l/io.html
4. https://mammon.github.io/Text/kernel_read.txt
5. https://stackoverflow.com/questions/1184274/read-write-files-within-a-linux-kernel-module
6. https://elixir.bootlin.com/
