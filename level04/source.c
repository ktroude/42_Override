/*
 * Decompiled / rewritten source for the exploited level04 binary.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/syscall.h>

void clear_stdin(void)
{
    int c;

    do {
        c = getchar();
        if (c == '\n')
            break;
    } while (c != EOF);
}

unsigned int get_unum(void)
{
    unsigned int n = 0;

    fflush(stdout);
    scanf("%u", &n);
    clear_stdin();
    return n;
}

void prog_timeout(int sig)
{
    (void)sig;
    _exit(1);
}

int enable_timeout_cons(void)
{
    signal(SIGALRM, prog_timeout);
    return alarm(60);
}

int main(int argc, char **argv, char **envp)
{
    pid_t child;
    int status;
    char buf[128];

    (void)argc;
    (void)argv;
    (void)envp;

    child = fork();
    memset(buf, 0, sizeof(buf));
    status = 0;

    if (child != 0)
    {
        while (1)
        {
            wait(&status);

            if (WIFEXITED(status) || WIFSIGNALED(status))
            {
                puts("child is exiting...");
                break;
            }

            /*
                On i386 Linux, ORIG_EAX is at offset 0x2c in the user area.
                Syscall 0xb is execve().
                The parent forbids execve, so normal /bin/sh shellcode is killed.
            */
            if (ptrace(PTRACE_PEEKUSER, child, 0x2c, 0) == 0xb)
            {
                puts("no exec() for you");
                kill(child, SIGKILL);
                break;
            }

            ptrace(PTRACE_CONT, child, 0, 0);
        }
    }
    else
    {
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        ptrace(PTRACE_TRACEME, 0, 0, 0);

        puts("Give me some shellcode, k");

        /*
            Vulnerability:
            gets() has no length limit.
            buf is only 128 bytes, so we can overflow the saved return address.
        */
        gets(buf);
    }

    return 0;
}
