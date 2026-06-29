# level03 notes:

Password calculation:
"Q}|u`sfg~sf{}|a3" XOR 0x12 = "Congratulations!"
0x1337d00d - 0x12 = 322424827

1. Copy level02 binary file to host machine
scp -P 7777 level03@127.0.0.1:/home/users/level03/level03 ./Lien-Override/BinaryfromISO/level03

2. Decompiler with this link:
https://dogbolt.org/?id=d2ff384a-ea37-4370-9ae5-4814118e5725

3. ssh level03@127.0.0.1 -p 7777
Enter password of level03: 
Hh74RPnuQ9sa5JAEXgNWCqz7sXGnh5J5M9KfPg3H

4. (python -c 'print(0x1337d00d - (ord("Q") ^ ord("C")))'; echo 'cat /home/users/level04/.pass'; echo 'exit') | ./level03

OR
(python -c 'print(0x1337d00d - (ord("Q") ^ ord("C")))'; cat) | ./level03
***********************************
*		level03		**
***********************************

5. id
uid=1003(level03) gid=1003(level03) euid=1004(level04) egid=100(users) groups=1004(level04),100(users),1003(level03)

6. whoami
level04

7. cat /home/users/level04/.pass
kgv3tkEb9h2mLkRsPkXRfc2mHbjMxQzvb2FrgKkf
