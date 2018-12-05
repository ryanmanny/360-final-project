#include "type.h"

int dev;

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;

int my_chmod(char *args[])
{
    char *pathname = args[0];
    char *mode_str = args[1];

    unsigned long mode;

    int ino;

    MINODE *wd, *mip;

    if (pathname[0] == '/')
    {
        wd = root;
        pathname++;
    }
    else
    {
        wd = running->cwd;
    }

    ino = getino(wd, pathname);
    mip = iget(wd->dev, ino);

    if (strlen(mode_str) == 3)
    {
        mode = strtoul(mode_str, NULL, 8);
    }
    else
    {
        printf("%s invalid mode\n", mode_str);
        return 1;
    }

    mip->INODE.i_mode = ((mip->INODE.i_mode >> 9) << 9) | mode;

    mip->dirty = 1;
    iput(mip);

    return 0;
}
