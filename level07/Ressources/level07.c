#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// vide le buffer stdin après un scanf (consomme jusqu'au \n ou EOF)
void clear_stdin(void)
{
    int c;
    do {
        c = getchar();
        if ((char)c == '\n')
            return;
    } while ((char)c != -1);
}

// lit un unsigned int depuis stdin (scanf %u + flush)
unsigned int get_unum(void)
{
    unsigned int num = 0;
    fflush(stdout);     // flush stdout pour garantir que le prompt s'affiche AVANT le scanf
    scanf("%u", &num);  // fgets mais pour un unsigned int
    clear_stdin();
    return num;
}

// ecrit un nombre à un index donné dans le tableau data
int store_number(unsigned int *data)
{
    unsigned int number;
    unsigned int index;

    printf(" Number: ");
    number = get_unum();
    printf(" Index: ");
    index = get_unum();

    // garde-fous :
    // - index multiple de 3 -> interdit ("reserved for wil")
    // - addresse qui commencent par 0xb7 -> interdit (bloque les adresses libc)
    if ((index % 3 == 0) || ((number >> 24) == 0xb7)) {
        puts(" *** ERROR! ***");
        puts("   This index is reserved for wil!");
        puts(" *** ERROR! ***");
        return 1;
    }

    // FAILLE
    //  aucune vérification que index < 100
    //  on peut écrire un int (soit 4 octets) n'importe ou
    data[index] = number;
    return 0;
}

// lit et affiche le nombre stocké à un index donné
int read_number(unsigned int *data)
{
    unsigned int index;

    printf(" Index: ");
    index = get_unum();

    // FAILLE
    //  même chose
    //  aucune vérification de taille sur index
    printf(" Number at data[%u] is %u\n", index, data[index]);
    return 0;
}

int main(int argc, char **argv, char **envp)
{
    unsigned int data[100];
    int result;
    char command[20];
    int i;

    // init le tableau data
    memset(data, 0, sizeof(data));

    // efface argv (les arguments de la ligne de commande)
    for (i = 0; argv[i] != NULL; i++)
        memset(argv[i], 0, strlen(argv[i]));

    // efface envp (toutes les variables d'environnement)
    // impossible de stocker un shellcode dans l'env
    for (i = 0; envp[i] != NULL; i++)
        memset(envp[i], 0, strlen(envp[i]));

    puts("----------------------------------------------------\n"
         "  Welcome to wil's crappy number storage service!   \n"
         "----------------------------------------------------\n"
         " Commands:                                          \n"
         "    store - store a number into the data storage    \n"
         "    read  - read a number from the data storage     \n"
         "    quit  - exit the program                        \n"
         "----------------------------------------------------\n"
         "   wil has reserved some storage :>                 \n"
         "----------------------------------------------------\n");

    // boucle principale : lit une commande et l'exécute
    while (1) {
        printf("Input command: ");
        result = 1;
        fgets(command, 20, stdin);

        // remplace le \n de l'entrée user par un \0
        command[strlen(command) - 1] = '\0';

        if (strcmp(command, "store") == 0) {
            result = store_number(data);
        }
        else if (strcmp(command, "read") == 0) {
            result = read_number(data);
        }
        else if (strcmp(command, "quit") == 0) {
            return 0; // le return de main -> c'est ici qu'on détourne via la saved EIP
        }

        // affiche le résultat
        if (result == 0)
            printf(" Completed %s command successfully\n", command);
        else
            printf(" Failed to do %s command\n", command);

        // remet command a zéro pour le prochain tour
        memset(command, 0, 20);
    }
}
