#include "type.h"

FS     filesystems[NMOUNT], *root_fs, *cur_fs;

int cd(int argc, char* args[])
{
    char *dirname = args[0];

    MINODE* mip = running->cwd;
    int ino = 0;

    if (dirname && dirname[0])
    {
        ino = getino(mip, dirname);
        
        if (ino > 0)
        {
            mip = iget(mip->fs, ino);

            if(S_ISDIR(mip->INODE.i_mode))
            {
                iput(running->cwd);
                running->cwd = mip;
                return 0;
            }
            else
            {
                printf("%s is not a directory\n", dirname);
                return 2;
            }
        }
        else
        {
            printf("%s does not exist\n", dirname);
            return 1;
        }
    }
    else  // Just cd into root
    {
        iput(running->cwd);
        running->cwd = root_fs->root;
        return 0;
    }
}
