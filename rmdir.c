#include "type.h"

MINODE minode[NMINODE];
MINODE *root;

int  dev;


int rm_child(MINODE* parent, char* name)
{
    DIR* dp,* prev;
    int curPos = 0; 
    char* cp;
    char buf[BLKSIZE];

    for(int i = 0; i < 12; i++)
    {
        if(!parent->INODE.i_block[i])
        {
            printf("No more blocks! %s not found!\n", name);
            return -1;
        }
        get_block(dev, parent->INODE.i_block[i], buf);
        cp = buf;
        dp = (DIR *) buf;

        while(cp < buf + BLKSIZE)
        {
            if (strncmp(dp->name, name, dp->name_len) == 0)
            {
                if(cp + dp->rec_len == buf + BLKSIZE)// First and only entry 
                {
                    char buf2[BLKSIZE];
                    put_block(parent->dev, parent->INODE.i_block, buf2);
                    bdalloc(parent->dev, parent->INODE.i_block[i]); // Boof the entire block
                    parent->INODE.i_size -= BLKSIZE; // Decrement the block by the entire size of a block
                    for(int j = i; j < 11; j++)
                    {
                        parent->INODE.i_block[j] =  parent->INODE.i_block[j+1];
                    }
                    parent->INODE.i_block[11] = 0;
                }
                else if(dp->rec_len > ideal_len(dp))
                {
                    prev->rec_len += dp->rec_len;
                }
                else
                {
                    // Entry is somewhere in the midle
                    int removedLength = dp->rec_len;
                    char* temp =buf;
                    DIR* lastDir = (DIR*) temp;
                    /// get to last entry
                    while (temp + lastDir->rec_len < buf + BLKSIZE)
                    {
                        temp += lastDir->rec_len;
                        lastDir = (DIR *) temp;
                    }

                    lastDir->rec_len += removedLength;
                    memcpy(dp, cp, BLKSIZE - curPos +1 - removedLength);
                }
                put_block(dev, parent->INODE.i_block[i], buf);
                parent->dirty = 1;
                return 1;
            }
            cp += dp->rec_len;//move to next entry
            curPos += dp->rec_len;//update current position
            prev = dp;//set prev
            dp = (DIR *)cp;
        }
    }
    return 0;

}

int my_rmdir(char* args[])
{
    char* path = args[0];
    char parent_path[128], filename[128], buf[BLKSIZE];

    int ino;
    MINODE * mip, *wd;

    // path is pathname we wanna create
    if (path[0] == '/')
    {
        // absolute path
        wd = root;
        dev = root->dev;
    }
    else
    {
        ///relative path
        wd =running->cwd;
        dev = running->cwd->dev;
    }

    strcpy(parent_path, path);
    strcpy(filename, path);

    strcpy(parent_path, dirname(parent_path));
    strcpy(filename, basename(filename));
    printf("parent: %s, child: %s path:%s\n", parent_path, filename, path);

    ino = getino(wd, path);

    if(ino == -1)
    {
        printf("not found!\n");
        return 0;
    }

    mip = iget(wd->dev, ino);

    if(running->uid != mip->INODE.i_uid && running->uid != 0)
    {
        printf("You do not have permission to remove this\n");
        return 0;
    }
    if (!S_ISDIR(mip->INODE.i_mode))
    {
        printf("Not a dir. Cannot be removed\n");
        iput(mip);
        return -1;
    }
    if (mip->INODE.i_links_count > 2)
    {
        printf("dir is not empty. cannot remove\n");
        iput(mip);
        return -1;
    }
    if(mip->INODE.i_links_count == 2)
    {
        DIR* dp;

        get_block(dev, mip->INODE.i_block[0], buf);
        char* cp;
        cp = buf;
        dp = (DIR*) buf;

       
        cp += dp->rec_len;
        dp = (DIR*) cp;

        /// second block of an empty disk should have rec_len of 1012. If it doesn't
        /// it's not empty
        if(dp->rec_len != 1012)
        {
            printf("Dir is not empty. Cannot be removed\n");
             iput(mip);
            return -1;
        }
    }

    for(int i = 0; i < 12; i++)
    {
        if(mip->INODE.i_block[i] == 0)
        {
            continue;
        }
        bdalloc(mip->dev, mip->INODE.i_block[i]);
    }
    idalloc(mip->dev, mip->ino);
    iput(mip);

    int pino = getino(wd, parent_path);
    if(ino == -1)
    {
        printf("not found!\n");
        return 0;
    }
    MINODE* pip = iget(wd->dev, pino);
    rm_child(pip, filename);
    pip->INODE.i_links_count--;
    pip->INODE.i_mtime = time(0L);
    pip->INODE.i_atime = pip->INODE.i_mtime;
    pip->dirty = 1;
    iput(pip);

    return 1;
}
