#include "type.h"

MINODE minode[NMINODE];
MINODE *root;

int dev;

int my_rmdir(char* args[])
{
    char* path = args[0];
    char parent_path[128], filename[128];

    int ino, pino;
    MINODE * mip, *wd;

    if (path[0] == '/')
    {
        wd = root;
        path++;
    }
    else
    {
        wd =running->cwd;
        dev = running->cwd->dev;
    }

    strcpy(parent_path, path);
    strcpy(filename, path);

    strcpy(parent_path, dirname(parent_path));
    strcpy(filename, basename(filename));
    
    ino = getino(wd, path);

    if (ino < 0)
    {
        printf("%s not found!\n", path);
        return 1;
    }

    mip = iget(wd->dev, ino);

    if (running->uid != mip->INODE.i_uid && running->uid != 0)
    {
        printf("Cannot rmdir: permission denied\n");
        return 0;
    }
    else if (!S_ISDIR(mip->INODE.i_mode))
    {
        printf("Cannot rmdir: %s is not a dir\n", path);
        iput(mip);
        return -1;
    }
    else if (mip->INODE.i_links_count > 2)
    {
        printf("Cannot rmdir: %s is not empty\n", path);
        iput(mip);
        return -1;
    }
    else if (mip->INODE.i_links_count == 2)
    {
        char buf[BLKSIZE];
        DIR* dp;
        char* cp;

        get_block(dev, mip->INODE.i_block[0], buf);
        
        cp = buf;
        dp = (DIR*) buf;

        cp += dp->rec_len;  // Get second entry in block
        dp = (DIR*) cp;

        // Second block of an empty disk will have rec_len of 1012
        if (dp->rec_len != 1012)
        {
            printf("Dir is not empty. Cannot be removed\n");
            iput(mip);
            return -1;
        }
    }

    truncate(mip);

    idalloc(mip->dev, mip->ino);
    iput(mip);

    // This will succeed because the getino for the child succeeded
    pino = getino(wd, parent_path);

    MINODE* pip = iget(wd->dev, pino);
    delete_entry(pip, filename);
    pip->INODE.i_links_count--;  // We just lost ".." from the deleted child
    pip->INODE.i_mtime = time(0L);
    pip->INODE.i_atime = pip->INODE.i_mtime;
    pip->dirty = 1;
    iput(pip);

    return 0;
}
