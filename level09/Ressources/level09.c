#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// structure qui contient le message, le username, et la taille pour strncpy
typedef struct s_message {
    char text[140];       // offset 0x00 : destination du message
    char username[40];    // offset 0x8C (140) : destination du username
    int  len;             // offset 0xB4 (180) : taille passée au strncpy (init à 140)
} t_message;

// Fonction cachée
//      jamais appelée dans le flux normal
//      si on redirige l'exécution ici, elle lit une commande et l'exécute
void secret_backdoor(void)
{
    char command[128];

    fgets(command, 128, stdin);
    system(command); // exécute ce qu'on tape -> /bin/sh = shell
}

// Fonction principale : gère les deux prompts (username + message)
void handle_msg(void)
{
    t_message msg;

    // init le username a zéro
    memset(&msg.username, 0, 40);

    // init la len a 140 (0x8C) = taille "normale" du strncpy
    msg.len = 0x8c;

    // demande et copie le username
    set_username(&msg);

    // demande et copie le message (utilise msg.len comme taille)
    set_msg(&msg);

    puts(">: Msg sent!");
    // au retour (retq), le CPU saute sur la saved RIP
    // si on l'a écrasée -> détournement vers secret_backdoor
}

// copie le message dans msg->text, en utilisant msg->len comme taille max
void set_msg(t_message *msg)
{
    char buffer[1024];

    // init le buffer local a zero
    memset(buffer, 0, 1024);

    puts(">: Msg @Unix-Dude");
    printf(">>: ");
    fgets(buffer, 1024, stdin); // lit le message (jusqu'a 1024 octets)

    // FAILLE
    //      strncpy copie msg->len octets du buffer vers msg->text
    //      si on a gonflé msg->len via set_username (1-byte overflow),
    //      le strncpy copie bien plus que 140 octets
    //      on déborde msg->text et écrase la saved RIP de handle_msg
    strncpy(msg->text, buffer, msg->len);
}

// copie le username dans msg->username, caractère par caractère
void set_username(t_message *msg)
{
    char buffer[140];
    int i;

    // init le buffer local a zero
    memset(buffer, 0, 128);

    puts(">: Enter your username");
    printf(">>: ");
    fgets(buffer, 128, stdin); // lit le username (jusqu'à 128 octets)

    // FAILLE
    //      la boucle copie jusqu'à 0x29 = 41 caractères
    //      msg->username fait 40 octets (offset 0x8C à 0xB3)
    //      msg->len est à l'offset 0xB4
    //      donc le 41e octet (index 40) DÉBORDE sur msg->len
    //      on contrôle la taille du strncpy dans set_msg
    for (i = 0; i < 41 && buffer[i] != '\0'; i++) {
        msg->username[i] = buffer[i];
    }

    printf(">: Welcome, %s", msg->username);
}

int main(void)
{
    puts("--------------------------------------------\n"
         "|   ~Welcome to l33t-m$n ~    v1337        |\n"
         "--------------------------------------------");

    handle_msg();

    return 0;
}
