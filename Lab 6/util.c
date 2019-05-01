       YOU MAY USE THESE FUNCTIONS in util.o

/************** util.c file ****************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

#include "type.h"

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern char gpath[128];
extern char *name[64];
extern int n;
extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;
extern char line[256], cmd[32], pathname[256];


int get_block(int dev, int blk, char *buf)
{
  // read disk block blk into char buf[BLKSIZE]
}   

int put_block(int dev, int blk, char *buf)
{
  // write buf[ ] to disk block blk
}   

int tokenize(char *pathname)
{
  // tokenize pathname into n components: name[0] to name[n-1];
  // n = number of token strings
}

MINODE *iget(int dev, int ino)
{
  // return minode pointer containing loaded INODE
  //(1). Search minode[ ] for an existing entry with the needed (dev, ino):
       if found: inc its refCount by 1;
                 return pointer to this minode;

  //(2). // needed entry not in memory:
       find a FREE minode (refCount = 0); Let mip-> to this minode;
       set its refCount = 1;
       set its dev, ino

  //(3). load INODE of (dev, ino) into mip->INODE:
       blk  = (ino-1) / 8 + inode_start;
       disp = (ino-1) % 8;
       get_block(dev, blk, buf);
       ip = (INODE *)buf + disp;

       mip->INODE = *ip;        // copy INODE to mip->INODE
       return mip;
}

int iput(MINODE *mip) // dispose a used minode by mip
{
 mip->refCount--;
 
 if (mip->refCount > 0) return;
 if (!mip->dirty)       return;
 
 // write INODE back to disk
 blk    =  (mip->ino - 1) / 8 + inode_start;
 offset =  (mip->ino - 1) % 8;

 get_block(mip->dev, blk, buf);

 ip = (INODE *)buf + offset;
 *ip = mip->INODE;

 put_block(mip->dev, blk, buf);

} 

// serach a DIRectory INODE for entry with given name
int search(MINODE *mip, char *name)
{
   // return ino of name if found; return 0 if NOT
}


// retrun inode number of pathname
int getino(int *dev, char *pathname)
{ 
   return ino of pathname if exist;
   return 0 if not
}
