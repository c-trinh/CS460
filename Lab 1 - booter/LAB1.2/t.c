/*******************************************************
*                      t.c file                        *
*******************************************************/
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

#define TRK 18
#define CYL 36
#define BLK 1024

#include "ext2.h"
typedef struct ext2_group_desc GD;
typedef struct ext2_inode INODE;
typedef struct ext2_dir_entry_2 DIR;
GD *gp;
INODE *ip;
DIR *dp;
u32 *up;

char buf1[BLK], buf2[BLK];
int color = 0x0A;

void prints(char *s)
{
  while (*s)
    putc(*s++);
}

void gets(char *s)
{
  while ((*s = getc()) != '\r')
  {
    putc(*s);
    s++;
  }
  *s = '\0';
}

int getblk(u16 blk, char *buf)
{
  readfd(blk / 18, ((blk) % 18) / 9, (((blk) % 18) % 9) << 1, buf);
}

u16 search(INODE *ip, char *name)
{
  int i;
  char c;
  DIR *dp;
  for (i = 0; i < 12; i++)
  {
    if ((u16)ip->i_block[i])
    {
      getblk((u16)ip->i_block[i], buf2);
      dp = (DIR *)buf2;
      while ((char *)dp < &buf2[BLK])
      {
        c = dp->name[dp->name_len];
        dp->name[dp->name_len] = 0;
        prints(dp->name); //Prints name
        putc(' ');
        if (strcmp(dp->name, name) == 0)
        {
          prints("\n\r");
          return ((u16)dp->inode);
        }
        dp->name[dp->name_len] = c;
        dp = (char *)dp + dp->rec_len;
      }
    }
  }
  error();
}

main()
{
  u32 *up;
  u16 i, ino, iblk;
  char c, temp[64], *name[2], file[64];

  name[0] = "boot";
  name[1] = file;

  prints("boot: ");
  gets(file);
  if (file[0] == 0)
    name[1] = "mtx";

  //prints("read block# 2 (GD)\n\r");
  getblk(2, buf1);

  gp = (GD *)buf1;

  // 1. WRITE YOUR CODE to get iblk = bg_inode_table block number
  iblk = (u16)gp->bg_inode_table;
  prints("\n\r");
  getblk(iblk, buf1);

  // 2. WRITE YOUR CODE to get root inode

  ip = (INODE *)buf1; //Begin Block
  ip += 1;

  for (i = 0; i < 2; i++)
  { //Will search for the given system name (in this case, mtx)
    ino = search(ip, name[i]) - 1;
    if (ino < 0)
      error();
    getblk(iblk + (ino / 8), buf1);
    ip = (INODE *)buf1 + (ino % 8);
  }

  // 3. WRITE YOUR CODE to step through the data block of root inode
  //prints("read data block of root DIR\n\r");

  if ((u16)ip->i_block[12]) //Read Indirect Block
    getblk((u16)ip->i_block[12], buf2);
  setes(0x1000); //load the blocks into memory starting from (segment) 0x1000
  for (i = 0; i < 12; i++)
  { //Gets DIRECT Blocks
    getblk((u16)ip->i_block[i], 0);
    inces(); // inc ES by 1KB/16 = 0x40
  }

  if ((u16)ip->i_block[12])
  { //Gets INDIRECT Blocks
    up = (u32 *)buf2;
    while (*up)
    {
      getblk((u16)*up, 0);
      up++;
      inces(); // inc ES by 1KB/16 = 0x40
    }
  }
  getc();
}