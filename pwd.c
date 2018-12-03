#include "type.h"

MINODE *root;

int rpwd(MINODE * wd)
{
    if (wd == root)
    {
        return 0;
    }

    char buf[BLKSIZE], dirname[BLKSIZE];
    
    int my_ino = 0, parent_ino = 0;
    get_block(dev, wd->INODE.i_block[0], buf);
    DIR* dp = (DIR *) buf;
    char* cp = buf;

    while(cp < buf + BLKSIZE)
    {
        strcpy(dirname, dp->name);
        dirname[dp->name_len] = '\0';
        
        if(strcmp(dirname, ".") == 0)
        {
            my_ino = dp->inode;
        }
        if(strcmp(dirname, "..") == 0)
        {
            parent_ino = dp->inode;
        }

        cp += dp->rec_len;
        dp = (DIR*) cp;
    }

    MINODE* pip = iget(dev, parent_ino);
    get_block(dev, pip->INODE.i_block[0], buf);
    dp = (DIR *) buf;
    cp = buf;

    while(cp < buf + BLKSIZE)
    {
        strncpy(dirname, dp->name, dp->name_len);
        dirname[dp->name_len] = '\0';
        
        if(dp->inode == my_ino)
        {
            break;
        }

        cp += dp->rec_len;
        dp = (DIR*) cp;
    }

    rpwd(pip);
    iput(pip);

    // Prints this part of cwd
    printf("/%s", dirname);
    return 0;
}

int pwd(MINODE *wd)
{
    if(wd == root)
    {
        printf("/");
    }
    else
    {
        rpwd(wd);
    }
    printf("\n");

    return 0;
}
