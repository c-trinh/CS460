#include <time.h>
#include <stdint.h>

enum FILEMODE
{
  R,
  W,
  RW,
  APPEND
};

int open_file()
{
  /// 1. ask for a pathname and mode to open:

  int mode;
  /*printf("0 = r\n1 = w\n2 = rw\n3 = append\nInput a mode:\n>> ");
  scanf("%d", &mode);*/

  if (strcmp(parameter, "r") == 0)
    mode = R;
  else if (strcmp(parameter, "w") == 0)
    mode = W;
  else if (strcmp(parameter, "rw") == 0)
    mode = RW;
  else if (strcmp(parameter, "append") == 0)
    mode = APPEND;

  int fd = myopen(pathname, mode);
  if (fd >= 0 && fd < 10)
    printf("Succesfully Opened FD @%d\n", fd);
}

int myopen(char *name, int mode)
{
  /// 2. get pathname's inumber:
  if (pathname[0] == '/')
    dev = root->dev; // root INODE's dev
  else
    dev = running->cwd->dev;
  int ino = getino(name);

  if (mode < 0 || mode > 3)
  {
    printf("[!] - ERROR: Out of bounds mode\n");
    return -1;
  }
  else if (!ino)
  {
    printf("[!] - ERROR: File does not exist\n");
    return -1;
  }

  /// 3. get its Minode pointer
  MINODE *mip = iget(dev, ino);

  /// 4. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.
  if (!S_ISREG(mip->INODE.i_mode))
  {
    printf("[!] ERROR: File is not REG\n");
    return -1;
  }

  for (int i = 0; i < 20; i++)
  {
    if (oft[i].mptr == mip && oft[i].mode != R)
    {
      printf("[!] ERROR: Current file already in OPEN mode.\n");
      return -1;
    }
  }

  for (int i = 0; i < 20; i++)
  {
  /// 6. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:
    if (oft[i].refCount == 0)
    {
      oft[i] = (OFT){.mode = mode, .refCount = 1, .mptr = mip, .offset = (mode == APPEND ? mip->INODE.i_size : 0)};
      if (mode == W)
      {
        mytruncate(mip);
        mip->INODE.i_size = 0;
      }
      for (int j = 0; j < NFD; j++)
      {
        if (!running->fd[j])
        {
          running->fd[j] = (oft + i);
          break;
        }
      }

      /// 8. update INODE's time field
      time_t current_time = time(0L);
      mip->INODE.i_atime = current_time;
      if (mode != R)
        mip->INODE.i_mtime = current_time;
      mip->dirty = 1;

      /// 9. return i as the file descriptor
      return i;
    }
  }
  return -1;
};

void close_file(int fd)
{
  OFT *op = running->fd[fd];
  op->refCount--;
  if (op->refCount == 0)
    iput(op->mptr);

  oft[fd].mptr = NULL;
  running->fd[fd] = NULL;
}

void myclose()
{
  close_file(atoi(pathname));
}

void lseek_file()
{
  int fd, position;
  printf("Enter fd and position: ");
  scanf("%d %d", &fd, &position);
}

void mylseek(int fd, int position)
{
  int original = running->fd[fd]->offset;
  running->fd[fd]->offset = (position <= running->fd[fd]->mptr->INODE.i_size && position >= 0) ? position : original;
}

int pfd()
{ ///This function displays the currently opened files
  printf("fd\tmode\toffset\tINODE\n");
  printf("---\t----\t------\t-----\n");
  for (int i = 0; i < NOFT; i++)
    if (oft[i].refCount)
    {
      char *mode;
      switch (oft[i].mode)
      {
      case R:
        mode = "R";
        break;
      case W:
        mode = "W";
        break;
      case RW:
        mode = "RW";
        break;
      case APPEND:
        mode = "A";
      }
      printf("%d\t%s\t%d\t%d\n", i, mode, oft[i].offset, oft[i].mptr->ino);
    }
}

/// Other Simple FILE DESCRIPTOR operations:

int dup(int fd)
{
  if (oft[fd].refCount == 0)
    return -1;
  for (int i = 0; i < NOFT; i++)
  {
    if (oft[i].refCount == 0)
    {
      oft[i] = oft[fd];
      oft[i].refCount = 1;
      return i;
    }
  }

  return -1;
}

int dup2(int fd, int gd)
{
  if (oft[fd].refCount == 0)
    return -1;
  if (oft[gd].refCount)
    close_file(gd);
  oft[gd] = oft[fd];
  oft[gd].refCount = 1;
}
