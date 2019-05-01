int argc;
char *argv[32]; // assume at most 32 tokens in cmdline

int exec(char *cmdline)
{
  int i, upa, usp;
  char *cp, kline[128], file[32], filename[32];
  PROC *p = running;
  strcpy(kline, cmdline);
  cp = kline;
  i = 0;
  
  while (*cp != ' ')
  {
    filename[i] = *cp;
    i++;
    cp++;
  }
  filename[i] = 0;
  file[0] = 0;
  upa = (int *)(p->pgdir[2048] & 0xFFFF0000);
  if (!load(filename, p))
  {
    parseArg(cmdline);
    return -1;
  }
  parseArg(cmdline);
  usp = upa + 0x100000 - 128;
  strcpy((char *)usp, kline);
  p->usp = (int *)VA(0x100000 - 128);
  for (i = 2; i < 14; i++)
    p->kstack[SSIZE - i] = 0;
  p->kstack[SSIZE - 1] = (int)VA(0);
  return (int)p->usp;
}

int parseArg(char *line)
{
  char *cp = line;
  char cmd[32];
  argc = 0;
  while (*cp != 0)
  {
    int i = 0;
    while (*cp == ' ')
      *cp++ = 0; // skip over blanks
    if (*cp != 0)
    {
      argv[argc++] = cp;       // pointed by argv[ ]
    }
    while (*cp != ' ' && *cp != 0)
    { // scan token chars
      cp++;
    }

    if (*cp != 0)
      *cp = 0; // end of token
    else
      break; // end of line
    printf("Argv[%d]: %s\n", argc-1, argv[argc-1]);
    cp++; // continue scan
  }
  argv[argc] = 0; // argv[argc]=0
}