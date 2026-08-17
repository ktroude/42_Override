#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// ecrit une entrée dans le fichier de log
// concatène le préfixe (ex: "Starting back up: ") avec le nom du fichier
// puis écrit le tout dans le log
void log_wrapper(FILE *logfile, char *prefix, char *filename)
{
    char logline[264];
    int prefix_len;

    // copie le préfixe dans le buffer
    strcpy(logline, prefix);

    // calcule la longueur du préfixe
    prefix_len = strlen(logline);

    // ajoute le nom du fichier a la suite, sans dépasser 254 octets au total
    // (0xfe = 254, moins la longueur du préfixe = place restante)
    snprintf(logline + prefix_len, 254 - prefix_len, filename);

    // remplace le \n s'il y en a un
    logline[strcspn(logline, "\n")] = '\0';

    // ecrit la ligne dans le fichier log
    fprintf(logfile, "LOG: %s\n", logline);
}

int main(int argc, char **argv)
{
    FILE *logfile;
    FILE *source;
    int dest_fd;
    char dest_path[104];
    char c = -1; // char lu (init a EOF)

    // verif du nombre d'args
    if (argc != 2) {
        printf("Usage: %s filename\n", argv[0]);
    }

    // ouvre le fichier de log (chemin relatif au repertoir courant -> ./)
    logfile = fopen("./backups/.log", "w");
    if (logfile == NULL) {
        printf("ERROR: Failed to open %s\n", "./backups/.log");
        exit(1);
    }

    // logue le début de la sauvegarde
    log_wrapper(logfile, "Starting back up: ", argv[1]);

    // ouvre le fichier SOURCE en lecture
    //    FAILLE
    //      lu avec les droits de level09
    //      -> peut lire des fichiers que level08 ne pourrait pas lire
    source = fopen(argv[1], "r");
    if (source == NULL) {
        printf("ERROR: Failed to open %s\n", argv[1]);
        exit(1);
    }

    // construit le chemin de DEST : "./backups/" + argv[1]
    //    FAILLE
    //      le chemin est construit bêtement par concaténation
    //      ->  si argv[1] = "home/users/level09/.pass" (relatif),
    //          la destination sera "./backups/home/users/level09/.pass"
    strncpy(dest_path, "./backups/", 11);
    strncat(dest_path, argv[1], 99 - strlen(dest_path));

    // crée le fichier de destination
    //    0xC1 = O_WRONLY | O_CREAT | O_EXCL
    //    0x1B0 = 0660 (perm read/write)
    dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_EXCL, 0660);
    if (dest_fd < 0) {
        printf("ERROR: Failed to open %s%s\n", "./backups/", argv[1]);
        exit(1);
    }

    // copie le contenu octet par octet : source -> destination
    while (1) {
        c = fgetc(source);
        if (c == -1) // EOF -> fin de la copie
            break;
        write(dest_fd, &c, 1);
    }

    // logue la fin de la sauvegarde
    log_wrapper(logfile, "Finished back up ", argv[1]);

    // ferme tout
    fclose(source);
    close(dest_fd);

    return 0;
}
