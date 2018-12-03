#include "type.h"

int dev;

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;

int cd(char* args[])
{
    char *dirname = args[0];

    MINODE* mip = running->cwd;
    int ino = 0;

    if (dirname && dirname[0])
    {
        ino = search(&mip->INODE, dirname);
        
        if (ino > 0)
        {
            mip = iget(dev, ino);

            if(S_ISDIR(mip->INODE.i_mode))
            {
                iput(running->cwd);
                running->cwd = mip;
                return 0;
            }
            else
            {
                printf("%s is not a directory", dirname);
                return 2;
            }
        }
        else
        {
            printf("%s does not exist", dirname);
            return 1;
        }
    }
    else
    {
        iput(running->cwd);
        running->cwd = root;
        return 0;
    }
}
