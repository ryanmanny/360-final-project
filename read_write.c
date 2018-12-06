#include "type.h"

extern PROC   proc[NPROC], *running;


int my_read(int fd, char buf[], int nbytes)
{
    char readbuf[BLKSIZE], *cp;

    OFT    *file = running->fd[fd];
    MINODE *mip  = file->mptr;
    
    file->refCount++;

    int count        = 0;

    int size         = file->mptr->INODE.i_size;
    
    int start_block  = file->offset / BLKSIZE;
    int start_byte   = file->offset % BLKSIZE; 
    int remain_block = BLKSIZE - start_byte;    // Number of bytes left in block (even if they're bad)
    int remain_file  = size - file->offset;     // Number of bytes left in file

    int i_block      = start_block;             // Block index
    int cur_block    = 0;                       // Actual block #

    int min;

    while (nbytes && remain_file)
    {
        min = nbytes;                           // Number of bytes to read
        if (remain_file < min)
            min = remain_file;
        if (remain_block < min)
            min = remain_block;

        cur_block = get_ith_block(mip, i_block);

        get_block(mip->dev, cur_block, readbuf);
        cp = readbuf + start_byte;
        start_byte = 0;

        memcpy(buf, cp, min);

        buf += min;
        count += min;
        file->offset += min;

        nbytes -= min;
        remain_file -= min;
        remain_block -= min;

        if (remain_block == 0)
        {
            i_block++;                           // Move to next block
            remain_block = BLKSIZE;
        }
    }

    file->refCount--;

    return count;  // Return actual number of bytes read
}

int my_write(int fd, char buf[], int nbytes)
{
    char writebuf[BLKSIZE], *cp;

    OFT    *file = running->fd[fd];
    MINODE *mip  = file->mptr;
    
    file->refCount++;

    int count        = 0;

    int start_block  = file->offset / BLKSIZE;
    int start_byte   = file->offset % BLKSIZE;
    int remain_block = BLKSIZE - start_byte;    // Number of bytes free in this block

    int i_block      = start_block;             // Block index
    int cur_block    = 0;                       // Actual block #

    int min;

    while (nbytes)
    {
        min = nbytes;                           // Number of bytes to read
        if (remain_block < min)
            min = remain_block;

        cur_block = get_ith_block(mip, i_block);
        get_block(mip->dev, cur_block, writebuf);

        cp = writebuf + start_byte;
        start_byte = 0;

        memcpy(buf, cp, min);

        put_block(mip->dev, cur_block, writebuf);

        buf += min;
        count += min;
        file->offset += min;

        nbytes -= min;
        remain_block -= min;

        if (remain_block == 0)
        {
            i_block++;                           // Move to next block
            remain_block = BLKSIZE;
        }
    }

    file->refCount--;
    mip->dirty = 1;                              // Mark mip dirty fucker       

    return count;
}
