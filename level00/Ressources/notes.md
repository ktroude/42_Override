# level00 notes

1. Create and Launch VM

Choose OverRide.iso file and configure parameters, in Network->Port Forwarding: put HOST Port: 7777, Guest Port: 4242
OverRide login: level00
Password: level00
RELRO           STACK CANARY      NX            PIE             RPATH      RUNPATH      FILE
Partial RELRO   No canary found   NX enabled    No PIE          No RPATH   No RUNPATH   /home/users/level00/level00

2. In the host machine:
ssh level00@127.0.0.1 -p 7777

level00@127.0.0.1's password: level00

RELRO           STACK CANARY      NX            PIE             RPATH      RUNPATH      FILE
Partial RELRO   No canary found   NX enabled    No PIE          No RPATH   No RUNPATH   /home/users/level00/level00

3. level00@OverRide:~$ ls -la
total 13
dr-xr-x---+ 1 level01 level01   60 Sep 13  2016 .
dr-x--x--x  1 root    root     260 Oct  2  2016 ..
-rw-r--r--  1 level01 level01  220 Sep 10  2016 .bash_logout
lrwxrwxrwx  1 root    root       7 Sep 13  2016 .bash_profile -> .bashrc
-rw-r--r--  1 level00 level00 3533 Sep 10  2016 .bashrc
-rwsr-s---+ 1 level01 users   7280 Sep 10  2016 level00
-rw-r--r--  1 level01 level01  675 Sep 10  2016 .profile

4. level00@OverRide:~$ ./level00
***********************************
* 	     -Level00 -		  *
***********************************
5. scp -P 7777 level00@127.0.0.1:/home/users/level00/level00 ./Lien-Override/BinaryfromISO/level00

6. Decompiler Explorer: Go to this link and attach file of level00 to get the password of 5276
https://dogbolt.org/?id=d2ff384a-ea37-4370-9ae5-4814118e5725

7. ./level00

8. Password:5276
Authenticated!

9. cat /home/users/level01/.pass
uSq2ehEGT6c9S24zbshexZQBXUGrncxn5sD5QfGL

10. su level01
paste password: uSq2ehEGT6c9S24zbshexZQBXUGrncxn5sD5QfGL

