#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void main(void)
{
    char buffer[100];
    int i;
    
    // lecture de l'entrée utilisateur (max 100 octets)
    fgets(buffer, 100, stdin);

    // parcour le buffer char par char
    for (i = 0; i < (int)strlen(buffer); i++) {
        
        // filtre majuscules -> minuscules
        // '@' = 0x40 (juste avant 'A') et '[' = 0x5b (juste après 'Z')
        // XOR 0x20 sur A-Z donne a-z (décale le bit 5)
        if (buffer[i] > '@' && buffer[i] < '[') {
            buffer[i] ^= 0x20; // en gros char c += 32;
        }
    }

    // FAILLE
    //  printf avec le buffer utilisateur comme format, les format string : %x, %n, %hn etc. seront interprétés
    printf(buffer);

    exit(0);
}
