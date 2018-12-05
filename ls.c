#include "type.h"

FS     filesystems[NMOUNT], *root_fs, *cur_fs;

// FUNCTIONS
int ls_file(MINODE *mip, char *filename)
{
    char entryname[512];
    char linkname[84];
    strcpy(entryname, filename);

    char *mask  = "rwxrwxrwx";
    char *bmask = "---------";
    int index = 0;
    u16 mode = mip->INODE.i_mode;

    char filetype;

    if (S_ISDIR(mip->INODE.i_mode))
        filetype = 'd';  // DIR
    else if (S_ISLNK(mip->INODE.i_mode))
    {
        filetype = 'l';  // LINK
        strcat(entryname, " -> ");
        getlink(mip, linkname);
        strcat(entryname, linkname);
    }
    else
        filetype = ' ';  // REG

    printf("%c", filetype);

    for (int shift = 8; shift >= 0; shift--)
    {
        if ((mode >> shift) & 1)
        {
            printf("%c", mask[index]);
        }
        else
        {
            printf("%c", bmask[index]);
        }
        index++;
    }

    printf(" %8d %s\n", mip->INODE.i_size, entryname);
    iput(mip);

    return 0;
}

int ls(char *args[])
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

            mip = iget(wd->fs, ino);
            
            if (temp[0] != '.')  // Skip hidden files
            {
                ls_file(mip, temp);
            }

            cp += dp->rec_len;  // Move to next record
            dp = (DIR *)cp;
        }
    }

    iput(pip);
    iput(mip);

    return 0;
}
