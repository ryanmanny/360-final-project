#include "type.h"

FS     filesystems[NMOUNT], *root_fs, *cur_fs;

int get_permissions(FS *fs, int ino, char *filename)
{
    MINODE *mip = iget(fs, ino);

    char *mask  = "rwxrwxrwx";
    char *bmask = "---------";
    int index = 0;
    u16 mode = mip->INODE.i_mode;

    char filetype;

    if (S_ISDIR(mip->INODE.i_mode))
        filetype = 'd';  // DIR
    else if (S_ISLNK(mip->INODE.i_mode))
        filetype = 'l';  // LINK
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
    printf(" ");
    return 0;
}

int my_stat(char* args[])
{
    struct stat myst;
    char* path = args[0];

    MINODE * mip, *wd;

    // path is pathname we wanna create
    if (path[0] == '/')
    {
        // absolute path
        wd = root_fs->root;
    }
    else
    {
        ///relative path
        wd =running->cwd;
    }

    int ino = getino(wd, path);
    mip = iget(wd->fs, ino);

    time_t a_time = (time_t )mip->INODE.i_atime;
    time_t c_time = (time_t )mip->INODE.i_ctime;
    time_t m_time = (time_t )mip->INODE.i_mtime;    

    printf("\nFile: '%s'\n", path);
    printf("Size: %d    Blocks: %d    %s\n", mip->INODE.i_size, mip->INODE.i_blocks, S_ISDIR(myst.st_mode)? "Dir file": S_ISLNK(mip->INODE.i_mode)? "LNK File": "Regular File");
    printf("Device: %d    Inode: %d    Links: %d\n", wd->dev, ino, mip->INODE.i_links_count);
    printf("Access: (");
    printf("0%d/", S_ISDIR(myst.st_mode)? 755: 644);
    get_permissions(wd->fs, ino, path);
    printf(")    Uid: %d    Gid: %d\n",  mip->INODE.i_uid, mip->INODE.i_gid);
    printf("Access: %s",ctime(&a_time));
    printf("Modify: %s",ctime(&m_time));
    printf("Change: %s\n",ctime(&c_time));
    iput(mip);

    return 0;
}

