#include "type.h"

FS     filesystems[NMOUNT], *root_fs, *cur_fs;

// FUNCTIONS
int ls_file(MINODE *mip, char *filename)
{
    char entryname[512];
    char linkname[84];
    strcpy(entryname, filename);

    char filetype = print_mode(mip->INODE.i_mode);

    if (filetype == 'l')  // Links are printed with the arrow
    {
        strcat(entryname, " -> ");
        getlink(mip, linkname);
        strcat(entryname, linkname);
    }

    printf(" %8d %s\n", mip->INODE.i_size, entryname);
    iput(mip);

    return 0;
}

int ls(int argc, char* args[])
{
    char *dirname = args[0];

    int ino;
    MINODE *wd, *pip, *mip;
    DIR *dp;
    char *cp, temp[256], dbuf[BLKSIZE];

    if (!dirname || !dirname[0])
    {
        wd = running->cwd;
        ino = running->cwd->ino;
    }
    else
    {
        if (dirname[0] == '/')
        {
            wd = root_fs->root;
            dirname++;
        }
        else
        {
            wd = running->cwd;
        }
        ino = getino(wd, dirname);
    }

    pip = iget(wd->fs, ino);
    

    if (S_ISDIR(pip->INODE.i_mode))
    {
        // TODO: Search the indirect blocks too
        for (int i = 0; i < 12; i++)
        {
            if (pip->INODE.i_block[i] == 0)
                break;

            get_block(wd->dev, pip->INODE.i_block[i], dbuf);
            dp = (DIR *) dbuf;
            cp = (char *) dbuf;

            while (cp < dbuf + BLKSIZE)
            {
                strncpy(temp, dp->name, dp->name_len);
                temp[dp->name_len] = '\0';

                if (temp[0] != '.')  // Skip hidden files
                {
                    mip = iget(wd->fs, dp->inode);
                    ls_file(mip, temp);
                }

                cp += dp->rec_len;  // Move to next record
                dp = (DIR *)cp;
            }
        }
    }
    else
    {
        printf("%s is not a dir\n", dirname);
    }

    iput(pip);
    iput(mip);

    return 0;
}
