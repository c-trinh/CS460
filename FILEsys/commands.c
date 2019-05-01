void write_file()
{
    int fd = atoi(pathname);
    if(running->fd[fd]->refCount == 0)
    {
        printf("File descriptor not open\n");
        return;
    }
    if(running->fd[fd]->mode == R)
    {
        printf("File is open for read\n");
        return;
    }

    mywrite(fd, parameter, strlen(parameter));
}

int mywrite(int fd, char buf[], int nbytes)
{
    int original_nbytes = nbytes;
    OFT * oftp = (running->fd[fd]);
    MINODE * mip = oftp->mptr;
    char * cq = buf;
    while(nbytes > 0)
    {
        int lbk = oftp->offset / BLKSIZE, startByte = oftp->offset % BLKSIZE, blk;

        if(lbk < 12)
        {
            if(mip->INODE.i_block[lbk] == 0)
                mip->INODE.i_block[lbk] = balloc(mip->dev);
            blk = mip->INODE.i_block[lbk];
        }
        else if(lbk >= 12 && lbk < 256 + 12)
        {
            if(mip->INODE.i_block[12] == 0)
            {
                mip->INODE.i_block[12] = balloc(mip->dev);
                zero_block(mip->dev, mip->INODE.i_block[12]);
            }
            uint32_t ibuf[256];
            get_block(mip->dev, mip->INODE.i_block[12], (char*)ibuf);
            blk = ibuf[lbk - 12];
            if(blk == 0)
            {
                if((blk = ibuf[lbk - 12] = balloc(mip->dev)) == 0)
                {
                    printf("[!] ERROR: Insufficient Disk Space.\n");
                    return original_nbytes - nbytes;
                }
                put_block(mip->dev, mip->INODE.i_block[12], (char*)ibuf);
            }
        }
        else
        {
            int indirect1 = (lbk - 256 - 12) / 256;
            int indirect2 = (lbk - 256 - 12) % 256;
            uint32_t ibuf[256];
            if(mip->INODE.i_block[13] == 0)
            {
                mip->INODE.i_block[13] = balloc(mip->dev);
                zero_block(mip->INODE.i_block[13], mip->INODE.i_block[13]);
            }
            get_block(mip->dev, mip->INODE.i_block[13], (char*)ibuf);
            if(ibuf[indirect1] == 0)
            {
                ibuf[indirect1] = balloc(mip->dev);
                zero_block(mip->dev, ibuf[indirect1]);
                put_block(mip->dev, mip->INODE.i_block[13], (char*)ibuf);
            }
            uint32_t ibuf2[256];
            get_block(mip->dev, ibuf[indirect1], (char*)ibuf2);
            if(ibuf2[indirect2] == 0)
            {
                ibuf2[indirect2] = balloc(mip->dev);
                zero_block(mip->dev, ibuf2[indirect2]);
                put_block(mip->dev, ibuf[indirect1], (char*)ibuf2);
            }
            blk = ibuf2[indirect2];
        }

        char wbuf[BLKSIZE];
        zero_block(mip->dev, blk);
        get_block(mip->dev, blk, wbuf);
        char * cp = wbuf + startByte;
        int remain = BLKSIZE - startByte;
        if(nbytes < remain)
            remain = nbytes;
        memcpy(cp, cq, remain);
        cq += remain;
        oftp->offset += remain;
        nbytes -= remain;
        mip->INODE.i_size += remain;
        put_block(mip->dev, blk, wbuf);
    }

    mip->dirty = 1;
    printf("wrote %d char into the file descriptor fd=%d\n", original_nbytes, fd);
    return original_nbytes;
}

/// (4). myread() tries to read nbytes from fd to buf[ ], and returns the actual number of bytes read.
int myread(int fd, char *buf, int nbytes)
{
    MINODE *mip = running->fd[fd]->mptr;
    OFT *op = running->fd[fd];
    int count = 0, avil = mip->INODE.i_size - op->offset;
    char *cq = buf;
    uint32_t dbuf[256];
    int blk;

    while(nbytes > 0 && avil > 0)
    {
        int lbk = op->offset / BLKSIZE, startByte = op->offset % BLKSIZE;

        if (lbk < 12)
        {
            blk = mip->INODE.i_block[lbk];
        }
        else if(lbk >= 12 && lbk < 268)
        {   /// Recieve blocks from indirect
            get_block(mip->dev,mip->INODE.i_block[12], (char*)dbuf);
            blk = dbuf[lbk - 12];
        }
        else
        {   /// Get to indirect blocks from double indirect
            get_block(mip->dev,mip->INODE.i_block[13], (char*)dbuf);
            get_block(mip->dev, dbuf[(lbk - 268) / 256], (char*)dbuf); // Get to indirect block
            blk = dbuf[(lbk-268) % 256];
        }

        char readbuf[BLKSIZE];
        get_block(mip->dev,blk, readbuf);
        cq = readbuf + startByte;
        int remainingBytes = BLKSIZE - startByte;
        if(nbytes < remainingBytes)
        {
            remainingBytes = nbytes;
        }
        memcpy((buf + count), cq, remainingBytes);
        op->offset += remainingBytes;
        count += remainingBytes;
        avil -= remainingBytes;
        nbytes -= remainingBytes;
        if (nbytes <= 0 || avil <= 0)
            break;
    }
    printf("FD: %d\tREAD-COUNT: %d\n", fd, count);
    return count;
}

void read_file()
{
    int fd = atoi(pathname), nbytes = atoi(parameter);
    char buf[nbytes + 1];
    if(running->fd[fd] == 0 || (running->fd[fd]->mode != R && running->fd[fd]->mode != RW))
    {
        printf("ERROR: FD %d is not open for read.\n", fd);
        return;
    }
    int ret = myread(fd, buf, nbytes);
    buf[ret] = 0;
    printf("%s\n", buf);
}

void mycat()
{
    int fd = myopen(pathname, R);
    if(fd == -1)
    {
        return;
    }
    char *buf = malloc(sizeof(char) * (running->fd[fd]->mptr->INODE.i_size + 1));
    myread(fd,buf,running->fd[fd]->mptr->INODE.i_size);
    buf[running->fd[fd]->mptr->INODE.i_size] = 0;
    printf("%s\n", buf);
    free(buf);
    close_file(fd);
}

void cp()
{
    printf("SRC: %s\n", pathname);
    printf("DST: %s\n", parameter);
    /// 1. fd = open src for READ;
    int fd = myopen(pathname, R);  // Opens src via pathname
    char temp[strlen(parameter) + 1];
    strcpy(temp, parameter);
    /// 2. gd = open dst for WR|CREAT;                
    int gd = myopen(parameter, W);   // Opens dst via parameter

    /// 3. write
    printf("[READING SRC]\n");
    char * buffer = malloc(sizeof(char) * running->fd[fd]->mptr->INODE.i_size + 1);
    myread(fd, buffer, running->fd[fd]->mptr->INODE.i_size);    // Read -> Source
    buffer[running->fd[fd]->mptr->INODE.i_size] = 0;

    printf("[WRITING DST]\n");
    mywrite(gd, buffer, running->fd[fd]->mptr->INODE.i_size);     // Write -> Parameter
    /*while( n=read(fd, buf[ ], BLKSIZE) ){
       mywrite(gd, buf, n);  // notice the n in write()
    }*/

    free(buffer);
    close_file(fd);
    close_file(gd);
}

void mymov()
{
    char * path1temp = strdup(pathname);
    link();
    strcpy(pathname, path1temp);
    unlink();
    free(path1temp);
}