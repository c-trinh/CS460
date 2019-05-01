/// init.c

#include "ucode.c"

int console, s0, s1;

int parent() // P1's code
{
    int pid, status;
    while (1)
    {
        prints("-- CNGINIT : wait for ZOMBIE child\n");
        pid = wait(&status);
        if (pid == console)
        { // if console login process died
            printf("-- INIT: forks a new console login\n");
            console = fork(); // fork another one
            if (console)
                continue;
            else
                exec("login /dev/tty0"); // new console login process
        }
        else if (pid == s0)
		{
			printf("CTINIT: forks a new login on serial port 0\n");
			s0 = fork();
			if (s0)
				continue;
			else
				exec("login /dev/ttyS0");
		}
		else if (pid == s1)
		{
			printf("CTINIT: forks a new login on serial port 1\n");
			s1 = fork();
			if (s1)
				continue;
			else
				exec("login /dev/ttyS1");
		}
        printf("-- CNGINIT: I just buried an orphan child proc %d\n", pid);
    }
}

main()
{
    int in = open("/dev/tty0", O_RDONLY);  // file descriptor 0
    int out = open("/dev/tty0", O_WRONLY); // for display to console
    
    printf("-- CNGINIT : fork a login proc on console\n");
    console = fork();

    /*
    P1 - INIT
    P2 - /dev/tty0
    P3 - /dev/ttyS0 
    P4 - /dev/ttyS1
    */

    if (console) // parent
        parent();
    else // child: exec to login on tty0
        exec("login /dev/tty0");
}