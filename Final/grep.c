//// grep.c

#include "ucode.c"

char buf[1024], tty[32];

int main(int argc, char *argv[])
{
    int in, out = 1, n_byt;
    char line[128], *pattern, c;

    gettty(tty);

    int tty_o = open(tty, O_WRONLY);

    if (argc < 2)
        return -1;

    if (argc == 2)
    {
        pattern = argv[1];
        in = 0;
    }
    else
    {   // contains filename
        pattern = argv[1];
        in = open(argv[2], O_RDONLY);
        if (in < 1)
        {
            return;
        }
    }

    int len = 0;
    int i = 0, j = 0;
    while (pattern[len] != '\0')
    {   /// Gets length of pattern
        len++;
    }
    int to_print = 0;
    while (1)
    {
        n_byt = read(in, buf, 1);
        if (n_byt < 1)
            break;

        line[i] = buf[0];

        if (buf[0] == 10)
        {
            line[++i] = '\n';
            if (to_print)
            {
                write(out, line, i);
                write(tty_o, "\r", 1);
            }
            to_print = 0;
            i = j = 0;
            memset(line, 0, 128);
        }
        else
        {
            if (line[i] == pattern[j])
            {
                j++;
                if (j == len && !to_print)
                {
                    to_print = 1;
                }
            }
            else
            {
                j = 0;
            }
            i++;
        }
    }
    close(in);
    close(out);
}