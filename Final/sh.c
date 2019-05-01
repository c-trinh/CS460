//// sh.c

#include "ucode.c"

char tokens[16][32];
int numtokens, i;

int contain(char *buf, char ch)
{ /// Scans Left -> Right to find 
  int len = 0;

  len = strlen(buf);
  while (len > 0)
  {
    if (buf[len] == ch)
    {
      return len;
    }
    len--;
  }
  return len;
}

int scan(char *cmd_line, char *head, char *tail, char ch)
{
  memset(head, 0, 128);
  memset(tail, 0, 128);
  char *cp = cmd_line;
  int h_len = contain(cmd_line, ch), t_len, i;
  //printf("[/] - h_len = %d\n", h_len);

  /// if '|' found in cmd_line
  if (h_len != 0)
  { /// Contains pipe
    //prints("[1] - Contains \'|\' symbol.");

    t_len = (strlen(cmd_line) - h_len) + 1;
    //printf("[/] - strlen(cmd_line) = %d\n", strlen(cmd_line));
    //printf("[/] - t_head = %d\n", h_len);

    i = 0;
    /// Splits Head + Tail
    while (*cp)
    { /// Iterates through cmd_line
      if (i < h_len)
      { /// Writes to Head
        *head++ = *cp++;
        //*tail++;
      }
      else if (i > h_len)
      { /// Writes to Tail
        //*head++;
        *tail++ = *cp++;
      }
      else
      { /// Skips over delimiter
        *cp++;
      }
      i++; // Keeps track of cp iteration
    }
    return 1;
  }
  else
  { /// Does not contain pipe
    //prints("[0] - Does not contain \'|\' symbol.");
    strcpy(head, cmd_line);
    return 0;
  }
}

int exec_command(char *cmd_line)
{
  char head[128], tail[128], *cmd = cmd_line;
  int fd, len;

  /// scan cmd_line for I/O (< > >>)
  /// do I/O direction
  if (scan(cmd_line, head, tail, '>'))
  { /// Contains > or >>, Handles stdout
    len = contain(head, '>');
    //printf("[/] - HEAD = %s, TAIL = %s, LEN = %d\n", head, tail, len);
    if (head[len] == '>')
    { // Contains '>>' for APPEND
      //prints("[0] = Found >> in command.\n");
      //printf("[!] - Deleteing %c char from head.\n", head[len]);
      head[len] = 0;
      close(1); // Close fd 1

      open(tail, O_WRONLY | O_APPEND);
      printf("HEAD = %s\n", head);
      exec(head);
    }
    // Contains '>' for CREAT
    prints("[0] = Found > in command.\n");
    close(1);

    open(tail, O_WRONLY | O_CREAT);
    exec(head);
  }
  if (len = scan(cmd, head, tail, '<'))
  { // Contains <, Handles stdin
    prints("[0] = Found < in command.\n");
    close(0);
    open(tail, O_RDONLY);
    exec(head);
  }
  exec(head);
}

int do_pipe(char *cmd_line, int *pd)
{
  int has_pipe = 0;
  char head[128], tail[128];
  int lpd[2], pid;

  if (pd)
  {                 // if Pipe passed in as WRITER
    close(pd[0]);   // closes pipe (READ)
    dup2(pd[1], 1); // redirect stdin to pipe READ end
    close(pd[1]);   // closes pipe (WRITE)
  }

  //has_pipe = scan(cmd_line, head, tail, '|');

  //printf("TAIL: %s\n", tail);
  //printf("HEAD: %s\n", head);

  //exec(cmd_line);

  if (scan(cmd_line, head, tail, '|'))
  {
    if (pipe(lpd) < 0)
    {
      printf("[!] - Pipe could not be opened.\n");
      exit(1);
    }

    pid = fork(); // fork a child to run pipe command

    if (pid)
    {                     /// Parent (as READER)
      close(lpd[1]);      // closes pipe (WRITE)
      dup2(lpd[0], 0);    // redirect stdin to pipe READ end
      close(lpd[0]);      // closes pipe (READ)
      exec_command(tail); // recursion call on the tail
    }
    else
    {                     /// Child (recursion w/ leftover)
      do_pipe(head, lpd); // redirect stdout to pipe WRITE end
    }
  }
  else
  { /// Leftover of command line
    exec_command(head);
  }
}

int main(int argc, char *argv[])
{
  char cmd[128], buf1[128], buf2[128];
  int status = 0;

  /// (in sh) it loops forever (until "logout")
  while (1)
  {
    memset(cmd, 0, 128); // Resets cmd buf each loop

    /// 1.) gets command lines from the user
    while (strcmp(cmd, "\0") == 0 || strcmp(cmd, "") == 0)
    { // if input is empty
      printf("Input a Command: ");
      gets(cmd);
    }

    /// 2.) executes the commands.

    /// Non-trivial commands (exit, cd)
    if (strcmp(cmd, "logout") == 0)
    { /// sycall to die;
      //prints("[!] - Exiting Shell...");
      exit(1);
    }

    if (cmd[0] == 'c' && cmd[1] == 'd')
    { /// Changes directory
      scan(cmd, buf1, buf2, ' ');
      printf("[0] - Changed to \'%s\' directory....\n", buf2);
      chdir(buf2);
      continue;
    }

    pid = fork(); // Fork a child sh process
    //printf("PID = %d\n", pid);

    if (pid == 0)
    { /// child sh - handles PIPE
      prints("[~] - Entered child sh...\n");
      do_pipe(cmd, 0);
    }
    else
    { /// parent sh
      prints("[~] - Entered parent sh...\n");
      pid = wait(&status);
    }
    //printc('\n');
  }
}