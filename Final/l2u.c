//// l2u

#include "ucode.c"

int main(int argc, char *argv[ ])
{  
	int fd_src, fd_dest, n, c;
	char buf[BLKSIZE], *cp;

	fd_src = open(argv[1], O_RDONLY);

    if (argc == 1)
	{
		while( read(0, &c, 1) > 0)
		{
			if ( c >= 'a' && c <= 'z')
				printc(c - 'a' + 'A');
			else
				printc(c);

			if (c == '\r')
				printc('\n');
		}
		exit(0);
	}

	if (fd_src < 0)
	{   /// Not enough arguments
		exit(1);
	}

	fd_dest = open(argv[2], O_WRONLY|O_CREAT);

	while( n = read(fd_src, buf, BLKSIZE) )
	{
		cp = buf;
		while(cp < buf + n)
		{
			if (*cp >= 'a' && *cp <= 'z')
			{
				*cp = *cp - 'a' + 'A';
			}
			cp++;
		}
		write(fd_dest, buf, n);
	}
	exit(0);
}