#include "type.h"

/********** globals *************/
FS     filesystems[NMOUNT], *root_fs, *cur_fs;

int tst_bit(char *buf, int bit)
{
    int i, j;
    i = bit/8; j=bit%8;
    if (buf[i] & (1 << j))
        return 1;
    return 0;
}

int set_bit(char *buf, int bit)
{
    int i, j;
    i = bit/8; j=bit%8;
    buf[i] |= (1 << j);

    return 0;
}

int clr_bit(char *buf, int bit)
{
    int i, j;
    i = bit/8; j=bit%8;
    buf[i] &= ~(1 << j);

    return 0;
}


int decFreeInodes(int dev)
{
    char buf[BLKSIZE];

    SUPER *sp;
    GD    *gp;

    // dec free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count--;
    put_block(dev, 2, buf);

    return 0;
}

int incFreeInodes(int dev)
{
    char buf[BLKSIZE];

    SUPER *sp;
    GD    *gp;

    // inc free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *) buf;
    sp->s_free_inodes_count++;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *) buf;
    gp->bg_free_inodes_count++;
    put_block(dev, 2, buf);

    return 0;
}

int ialloc(FS *fs)
{
    int  i;
    char buf[BLKSIZE];

    // read inode_bitmap block
    get_block(fs->dev, fs->imap, buf);

    for (i=0; i < fs->ninodes; i++)
    {
        if (!tst_bit(buf, i))  // Inode not already allocated
        {
            set_bit(buf,i);
            decFreeInodes(fs->dev);

            put_block(fs->dev, fs->imap, buf);

            return i+1;
        }
    }
    printf("ialloc(): no more free inodes\n");
    return 0;
}

int balloc(FS *fs)
{
    int  i;
    char buf[BLKSIZE];

    // read inode_bitmap block
    get_block(fs->dev, fs->bmap, buf);

    for (i=0; i < fs->nblocks; i++)
    {
        if (!tst_bit(buf, i))  // Block not already allocated
        {
            set_bit(buf,i);
            decFreeInodes(fs->dev);

            put_block(fs->dev,  fs->bmap, buf);

            return i+1;
        }
    }
    printf("balloc(): no more free blocks\n");
    return 0;
}

void idalloc(FS *fs, int ino)
{
    char buf[BLKSIZE];
    if (ino > fs->ninodes)
    {
        printf("Error: ino %d out of range\n", ino);
        return;
    }
    get_block(fs->dev, fs->imap, buf);
    clr_bit(buf, ino-1);
    put_block(fs->dev, fs->imap, buf);
    incFreeInodes(fs->dev);
}

void bdalloc(FS *fs, int bno)
{
    char buf[BLKSIZE];
    if(bno > fs->nblocks)
    {
        printf("Error: ino %d out of range\n", bno);
        return;
    }
    get_block(fs->dev, fs->bmap, buf);
    clr_bit(buf, bno-1);
    put_block(fs->dev, fs->bmap, buf);
    incFreeInodes(fs->dev);
}
