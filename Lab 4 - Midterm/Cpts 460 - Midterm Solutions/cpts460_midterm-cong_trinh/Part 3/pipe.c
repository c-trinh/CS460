#define NPIPE 10
#define PSIZE 8

typedef struct pipe
{
  char buf[8];    // circular data buffer
  int head, tail; // circular buf index
  int data, room; // number of data & room in pipe
} PIPE;

PIPE pipe;

int show_pipe()
{
  PIPE *p = &pipe;
  int i;
  printf("----------------------------------------\n");
  printf("room=%d data=%d buf=", p->room, p->data);
  for (i = 0; i < p->data; i++)
    kputc(p->buf[p->tail + i]);
  printf("\n");
  printf("----------------------------------------\n");
}
int kpipe()
{
  int i;
  PIPE *p = &pipe;
  p->head = p->tail = 0;
  p->data = 0;
  p->room = PSIZE;
}

int read_pipe(PIPE *p, char *buf, int n)
{
  int ret;
  char c;

  if (n <= 0)
    return 0;
  show_pipe();

  while (n)
  {
    printf("reader %d reading pipe\n", running->pid);
    ret = 0;
    while (p->data)
    {
      *buf = p->buf[p->tail++];
      p->tail %= PSIZE;
      buf++;
      ret++;
      p->data--;
      p->room++;
      n--;
      if (n <= 0)
        break;
    }
    show_pipe();
    if (ret)
    {
      kwakeup(&p->room);
      return ret;
    }
    // No data found from Pipe, must Sleep
    printf("reader %d sleep for data\n", running->pid);
    kwakeup(&p->room);
    ksleep(&p->data);
    continue;
  }
}

int write_pipe(PIPE *p, char *buf, int n)
{
  char c;
  int ret = 0;
  show_pipe();
  while (n)
  {
    printf("writer %d writing pipe\n", running->pid);
    while (p->room)
    {
      p->buf[p->head++] = *buf; // write a byte to pipe;
      p->head %= PSIZE;
      buf++;
      ret++;
      p->data++;
      p->room--;
      n--;
      if (n <= 0)
      {
        show_pipe();
        kwakeup(&p->data);
        return ret;
      }
    }
    show_pipe();
    printf("writer %d sleep for room\n", running->pid);
    kwakeup(&p->data);
    ksleep(&p->room);
  }
}

int pipe_reader()
{
  char line[128];
  int nbytes, n;
  PIPE *p = &pipe;
  printf("proc %d as pipe reader\n", running->pid);

  while (1)
  {
    /*if (p->data = 0)
  { //No data nor writer
    printf("NO MORE DATA - TERMINATE\n");
    return 0;
  }*/

    printf("input nbytes to read : ");
    nbytes = geti();
    if (nbytes == 0)
    {
      kwakeup(&p->room);
      kexit(running->pid);
    }
    n = read_pipe(p, line, nbytes);
    line[n] = 0;
    printf("Read n=%d bytes : line=%s\n", n, line);
  }
}

int pipe_writer()
{
  char line[128];
  int nbytes, n;
  PIPE *p = &pipe;
  printf("proc %d as pipe writer\n", running->pid);

  while (1)
  {
    printf("input a string to write : ");
    kgets(line); // Gets input from user

    if (strcmp(line, "") == 0)
    {
      kexit(running->pid);
      continue;
    }
    nbytes = strlen(line);
    printf("nbytes=%d buf=%s\n", nbytes, line);
    n = write_pipe(p, line, nbytes);
    printf("wrote n=%d bytes\n", n);
  }
}
