//// cat.c

#include "ucode.c"

int main(int argc, char *argv[])
{
    int fd, n, i, terminal;
    char c, c2, buf[BLKSIZE], ret = '\r';
    char *tty;

    if (argc < 2)
    {   /// use stdin
        while (gets(buf))
        {
            write(2, buf, (int)strlen(buf));
            write(2, "\n\r", 2);
        }
    }
    else if (argc >= 2)
    {   /// filename provided
        fd = open(argv[1], O_RDONLY);

        if (fd < 0)
            exit(1);

        while ((n = read(fd, buf, BLKSIZE)) > 0)
        {   /// Read through fd
            for (i = 0; i < n; i++)
            {
                write(1, &(buf[i]), 1); // Write to fd 1 the character

                if (buf[i] == '\n')
                    write(2, &ret, 1);  // Write return for new line
            }

            for (i = 0; i < BLKSIZE; i++)
                buf[i] = 0;
        }
    }

    close(fd);  // Close file descriptor
    exit(0);
}