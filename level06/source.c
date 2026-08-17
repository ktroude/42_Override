#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ptrace.h>

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

// vérifie si le serial correspond au login -> renvoie 0 si OK, 1 si échec
int auth(char *login, unsigned int serial)
{
    int len;
    int i;
    unsigned int hash;

    // retire le \n du login
    login[strcspn(login, "\n")] = '\0';

    // login trop court -> échec
    len = strnlen(login, 32);
    if (len < 6)
        return 1;

    // anti-debug : ptrace détecte si un debugger est attaché
    if (ptrace(PTRACE_TRACEME) == -1) {
        puts("\033[32m.---------------------------.");
        puts("\033[31m| !! TAMPERING DETECTED !!  |");
        puts("\033[32m'---------------------------'");
        return 1;
    }

    // calcul du hash (serial attendu) à partir du login
    //    - seed : le 4e caractère XOR 0x1337, plus 0x5eeded
    //    - accumulation : pour chaque caractère, (char ^ hash) % 1337
    hash = (login[3] ^ 0x1337) + 0x5eeded;

    for (i = 0; i < len; i++) {
        if (login[i] < ' ')        // char de controle -> échec
            return 1;
        hash += (login[i] ^ hash) % 0x539;   // 0x539 = 1337
    }

    // comparaison : le serial saisi doit correspondre au hash calculé
    if (serial == hash)
        return 0;       // success : auth renvoie 0 -> accès au shell
    else
        return 1;
}

int main(void)
{
    char login[32];
    unsigned int serial;
    int result;

    puts("***********************************");
    puts("*\t\tlevel06\t\t  *");
    puts("***********************************");

    // lecture du login
    printf("-> Enter Login: ");
    fgets(login, 32, stdin);

    puts("***********************************");
    puts("***** NEW ACCOUNT DETECTED ********");
    puts("***********************************");

    // lecture du serial (entier)
    printf("-> Enter Serial: ");
    serial = get_unum();

    // vérif
    result = auth(login, serial);

    // si auth renvoie 0 -> shell avec les droits level07
    if (result == 0) {
        puts("Authenticated!");
        system("/bin/sh");
    }

    return (result != 0);
}