#include "type.h"

FS     filesystems[NMOUNT], *root_fs, *cur_fs;

// FUNCTIONS
int rpwd(MINODE *wd)
{
    char buf[BLKSIZE], dirname[BLKSIZE];
    int my_ino, parent_ino;

    DIR* dp;
    char* cp;

    MINODE* pip;  // Parent MINODE

    if (wd == root_fs->root)
    {
        return 0;
    }

    // Get dir block of cwd
    get_block(wd->dev, wd->INODE.i_block[0], buf);
    dp = (DIR *) buf;
    cp = buf;

    // Searches through cwd for cwd and parent ino
    // TODO: Replace with search?
    while(cp < buf + BLKSIZE)
    {
        strcpy(dirname, dp->name);
        dirname[dp->name_len] = '\0';
        
        if(!strcmp(dirname, "."))
        {
            my_ino = dp->inode;
        }
        if(!strcmp(dirname, ".."))
        {
            parent_ino = dp->inode;
        }

        cp += dp->rec_len;
        dp = (DIR*) cp;
    }

    pip = iget(wd->fs, parent_ino);
    get_block(wd->dev, pip->INODE.i_block[0], buf);
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

int pwd(char *args[])
{
    MINODE *wd = running->cwd;

    if(wd == root_fs->root)
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
