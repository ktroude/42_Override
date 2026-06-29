#include <stdio.h>
#include <stdlib.h>

/*
 * Decompiled / rewritten source for the exploited level00 binary.
 */
int main(void)
{
    int password = 0;

    puts("***********************************");
    puts("* \t     -Level00 -\t\t  *");
    puts("***********************************");
    printf("Password:");
    scanf("%d", &password);

    if (password == 5276)
    {
        puts("\nAuthenticated!");
        system("/bin/sh");
        return 0;
    }

    puts("\nInvalid Password!");
    return 1;
}
