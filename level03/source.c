/*
 * Reconstructed source for OverRide level03. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void clear_stdin(void)
{
    int c;

    do {
        c = getchar();
        if (c == '\n')
            break;
    } while (c != EOF);
}

static unsigned int get_unum(void)
{
    unsigned int result = 0;

    fflush(stdout);
    scanf("%u", &result);
    clear_stdin();
    return result;
}

static void prog_timeout(void)
{
    _exit(1);
}

static int decrypt(char key)
{
    char encrypted[] = "Q}|u`sfg~sf{}|a3";
    const char expected[] = "Congratulations!";
    size_t i;

    for (i = 0; encrypted[i] != '\0'; ++i)
        encrypted[i] ^= key;

    if (strcmp(encrypted, expected) == 0)
        return system("/bin/sh");

    puts("\nInvalid Password");
    return 0;
}

static int test(unsigned int input, unsigned int magic)
{
    int diff = (int)(magic - input);

    switch (diff) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
            return decrypt((char)diff);
        default:
            return decrypt((char)rand());
    }
}

int main(void)
{
    unsigned int password;

    srand((unsigned int)time(NULL));

    puts("***********************************");
    puts("*\t\tlevel03\t\t**");
    puts("***********************************");

    printf("Password:");
    password = get_unum();

    test(password, 0x1337d00d);
    return 0;
}
