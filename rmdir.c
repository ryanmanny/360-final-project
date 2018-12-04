#include "type.h"

MINODE minode[NMINODE];
MINODE *root;

int  dev;

int remove_dir(char* args[])
{
    char* path = args[0];
    char parent_path[128], filename[128], buf[BLKSIZE];

    int ino;
    MINODE * mip;
    // path is pathname we wanna create
    if (path[0] == '/')
    {
        // absolute path
        dev = root->dev;
    }
    else
    {
        ///relative path
        dev = running->cwd->dev;
    }

    ino = getino(path);
    mip = iget(dev, ino);

    if(running->uid != mip->INODE.i_uid && running->uid != 0)
    {
        printf("You do not have permission to remove this\n");
        return 0;
    }
    if (!S_ISDIR(mip->INODE.i_mode))
    {
        printf("Not a dir. Cannot be removed\n");
        return 0;
    }
    if(mip->INODE.i_links_count >= 2)
    {
        DIR* dp;

        get_block(dev, ino, buf);
        dp = (DIR*) buf;

        char* cp;
        cp += dp->rec_len;
        dp = (DIR*) cp;

        /// second block of an empty disk should have rec_len of 1012. If it doesn't
        /// it's not empty
        if(dp->rec_len< 1012)
        {
            printf("Dir is not empty. Cannot be removed\n");
            return 0;
        }
    }
}