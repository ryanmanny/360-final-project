#include "type.h"

FS     filesystems[NMOUNT], *root_fs, *cur_fs;

int my_touch(char* args[])
{
    char* path = args[0];

    MINODE * mip, *wd;

    // path is pathname we wanna create
    if (path[0] == '/')
    {
        // absolute path
        wd = root_fs->root;
        path++;
    }
    else
    {
        ///relative path
        wd =running->cwd;
    }

    int ino = getino(wd, path);

    if (ino == -1)
    {
        ///create file
        my_creat(args);
    }
    else
    {
        mip = iget(wd->fs, ino);

        mip->INODE.i_mtime = time(0L);
        mip->INODE.i_atime = mip->INODE.i_mtime;
    }

    return 0;
}
