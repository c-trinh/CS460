//// login.c

#include "ucode.c"

int in, out, err;
char username[128], password[128];

int tokenize(char *line, char *buf[], char ch)
{
  int len = 0;
  char *cp = line;

  /// Taken from 'token()' in crt0.c
  while (*cp != 0)
  {
    /// Trims whitespace
    while (*cp == ' ')
      *cp++ = 0;

    /// Scans string
    if (*cp != 0)
      buf[len++] = cp;

    /// Splits lines
    while (*cp != ch && *cp != 0)
      cp++;

    if (*cp != 0)
      *cp = 0;
    else
      break;

    cp++;
  }
  return len;
}

int main(int argc, char *argv[])
{
  int i, pid = getpid();
  char *lines[128], *acc_info[128], file_read[1024];
  
  // (1). close file descriptors 0,1 inherited from INIT.
  close(0);
  close(1);
  prints("[0] - Successfully Closed FD 0, 1.\n");

  // (2). open argv[1] 3 times as in(0), out(1), err(2).
  in = open(argv[1], 0);
  out = open(argv[1], 1);
  err = open(argv[1], 2);
  prints("[0] - Successfully Opened argv[1] in(0), out(1), err(2).\n");

  // (3). set tty username string in PROC.tty
  fixtty(argv[1]);

  printf("argc = %d\n", argc);
  for (i = 0; i < argc; i++)
    printf("argv[%d] = %s\n", i, argv[i]);

  while (1)
  {
    printf("CNG: PROC %d running login program\n", pid);
    // (4). open /etc/passwd file for READ;
    int file = open("/etc/passwd", O_RDONLY);

    // (5). Get username, password & tokenize
    printc('\n');
    printf(">> LOGIN: ");
    gets(username);

    printf(">> PASSWORD: ");
    gets(password);

    read(file, file_read, 1024);

    //prints(file_read); // Prints what was read in passwd for DEBUGGING

    int len = tokenize(file_read, lines, '\n'); // Get individual lines from file read buf

    for (i = 0; i < len; i++)
    {
      tokenize(lines[i], acc_info, ':'); // Splits individual lines to get info

      // (6). if (user has a valid account){
      int username_valid = strcmp(username, acc_info[0]) == 0;
      int password_valid = strcmp(password, acc_info[1]) == 0;

      if (username_valid && password_valid)
      { //if username and password both valid, account is VALID

        // (7-1). change uid, gid to user's uid, gid; // chuid()
        chuid(atoi(acc_info[2]), atoi(acc_info[3])); // chuid( uid, gid )

        // (7-2).change cwd to user's home DIR // chdir()
        chdir(acc_info[5]);

        printf("[0] - CNG LOGIN SUCCESSFUL.\nuser:\t%s\npass:\t%s\ngid:\t%d\nuid:\t%d\nhome:\t%s\nshell:\t%s\n\n",
               acc_info[0], acc_info[1], atoi(acc_info[2]), atoi(acc_info[3]), acc_info[5], acc_info[6]);

        // (7-3). close opened /etc/passwd file // close()
        close(file);

        // (8). exec to program in user account // exec()
        exec(acc_info[6]); // Execute Shell

        break;
      }
    }
    prints("[!] - Wrong Login.\n");
  }
  //printf("PROC %d exit\n", pid);
}