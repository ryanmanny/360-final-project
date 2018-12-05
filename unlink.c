#include "type.h"

int my_unlink(char *args[])
{
    int ino, pino;

    char *pathname = args[0];
    char filename[128], parent_path[128];

    MINODE *wd, *mip, *pip;

    if (pathname[0] == '/')
    {
        wd = root;
        pathname++;
    }
    else
    {
        wd = running->cwd;
    }

    strcpy(parent_path, pathname);
    strcpy(parent_path, dirname(parent_path));

    strcpy(filename, pathname);
    strcpy(filename, basename(filename));

    ino = getino(wd, pathname);
    mip = iget(wd->dev, ino);

    pino = getino(wd, parent_path);
    pip = iget(wd->dev, pino);

    if (S_ISDIR(mip->INODE.i_mode))
    {
        printf("Can't unlink: %s is a dir\n", pathname);
        return 1;
    }
    else
    {
        mip->INODE.i_links_count--;
        if (mip->INODE.i_links_count == 0)
        {
            if (S_ISREG(mip->INODE.i_mode))
            {
                truncate(mip);
            }
            else if (S_ISLNK(mip->INODE.i_mode))
            {
                // I don't think we need to do anything
            }
            idalloc(mip->dev, mip->ino);
        }

        // Remove the dirent from the parent
        delete_entry(pip, filename);
    }
    
    iput(mip);
    iput(pip);

    return 0;
}
