//// cp.c

#include "ucode.c"

int main(int argc, char *argv[])
{
    char buf[BLKSIZE];
    int fd_src, fd_dest;
    int n, bytes = 0;

    if (argc < 3)
    {   /// No source/destination file provided
        exit(1);
    }

    fd_src = open(argv[1], O_RDONLY);               // Opens source for input

    if (fd_src < 0)
    {   /// open src file failed
        exit(2);
    }

    fd_dest = open(argv[2], O_WRONLY | O_CREAT);    // Opens destination for output

    if (fd_dest < 0)
    {   /// Open dest file failed
        exit(3);
    }

    while (n = read(fd_src, buf, BLKSIZE))
    {   /// Writes src -> dest per char
        write(fd_dest, buf, n);
    }

    close(fd_src);
    close(fd_dest);
    exit(0);
}