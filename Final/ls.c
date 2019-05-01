//// ls.c

#include "ucode.c"

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int ls_file(char *fname)
{
    struct stat fstat, *sp = &fstat;

    int r, i;
    char sbuf[4096];

    r = stat(fname, sp);

    if (sp->st_mode == 16877)
    {   /// 16877 = ISREG, is a Directory
        printc('d');
    }
    else
    {   /// 33060 = ISDIR, is a File
        printc('-');
    }

    // bit permissions
    for (i = 8; i >= 0; i--)
    {
        /// Prints x, r, w
        if (sp->st_mode & (1 << i))
            printc(t1[i]);  // Permission Allowed
        else
            printc(t2[i]);  // Permission Denied
    }

    printf(" %d ", sp->st_nlink);   // link count
    printf(" %d", sp->st_uid);      // uid
    printf(" %d", sp->st_gid);      // gid
    printf(" %d", sp->st_size);     // filesize
    printf(" %s\n\r", fname);       // filename
}

void ls_dir(char *path)
{
    prints("---- CNG: Cong's LS Function ----\n");

    char read_buf[BLKSIZE], name[128];

    int fd = open(path, O_RDONLY);

    chdir(path);
    //printf("[/] - Path = %d\n", path);

    int n = read(fd, read_buf, BLKSIZE);
    //printf("[/] - read_buf = %d\n", read_buf);

    DIR *dp = (DIR *)read_buf;  // directory pointer

    char *cp = read_buf;
    while (cp < read_buf + BLKSIZE)
    {
        strcpy(name, dp->name);
        strcat(name, "\0");
        ls_file(name);

        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    close(fd);
}

main(int argc, char *argv[])
{
    struct stat fstat, *sp = &fstat;

    char path[128];

    getcwd(path);
    printf("argc = %d\n", argc);

    if (argc == 1)
    {   /// Print current directory
        prints("[0] - ls for CWD.\n");
        ls_dir(path);
    }

    else if (argc > 1)
    {
        int r = stat(argv[1], sp);

        printf("[/] - st_mode = %d\n", sp->st_mode);

        if (sp->st_mode == 33060) /// 33060 = ISREG
        {                         /// If arg is File
            prints("[1] - File is Regular.\n");
            ls_file(argv[1]); // ls file
        }
        else if (sp->st_mode == 16877) /// 16877 = ISDIR
        {                              /// If arg is Folder
            prints("[0] - File is not Regular.\n");
            ls_dir(argv[1]); // ls dir
        }
        else
        {
            prints("[!] - ERROR: No file type detected.\n");
        }
    }
}