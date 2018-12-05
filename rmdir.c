#include "type.h"

MINODE minode[NMINODE];
MINODE *root;

int dev;

int my_rmdir(char* args[])
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

    strcpy(parent_path, path);
    strcpy(filename, path);

    strcpy(parent_path, dirname(parent_path));
    strcpy(filename, basename(filename));
    printf("parent: %s, child: %s path:%s\n", parent_path, filename, path);

    ino = getino(root, path);

    if(ino == -1)
    {
        printf("not found!\n");
        return 0;
    }

    mip = iget(dev, ino);



    int pino = getino(root, parent_path);

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
    // if(mip->refCount > 0)
    // {
    //     printf("Dir is busy. Cannot be removed.\n");
    //      iput(mip);
    //     return -1;
    // }

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

    ino = getino(root, parent_path);
    if(ino == -1)
    {
        printf("not found!\n");
        return 0;
    }
    MINODE* pip = iget(dev, ino);
    delete_entry(pip, filename);
    pip->INODE.i_links_count--;
    pip->INODE.i_mtime = time(0L);
    pip->INODE.i_atime = pip->INODE.i_mtime;
    pip->dirty = 1;
    iput(pip);

    return 1;

    /// CONNTINUE HERE
}
