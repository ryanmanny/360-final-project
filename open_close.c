#include "type.h"
OFT oft[NOFT];
FS     filesystems[NMOUNT], *root_fs, *cur_fs;

#define READ 0
#define WRITE 1
#define READWRITE 2
#define APPEND 3

int my_open(char* path, char* modeStr)
{
    int fd, ino;
    MINODE* wd, *mip;
    
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

    ///DETERMINE MODE WE ARE OPENING IN
    int mode = 0;
    if(strcmp(modeStr, "R") == 0 || strcmp(modeStr, "0") == 0)
    {
        mode = READ;
    }
    else if(strcmp(modeStr, "W") == 0 || strcmp(modeStr, "1") == 0)
    {
        mode = WRITE;
    }
    else if(strcmp(modeStr, "RW") == 0 || strcmp(modeStr, "2") == 0)
    {
        mode = READWRITE;
    }
    else if(strcmp(modeStr, "APPEND") == 0 || strcmp(modeStr, "3") == 0)
    {
        mode = APPEND;
    }
    else
    {
        printf("%s is an invalid mode\n", modeStr);
        return -1;
    }

    char path_cpy[256];
    strcpy(path_cpy, path);
    ino = getino(wd, path_cpy);

    if (ino < 0)
    {
        if (mode != READ)
        {
            // Create file
            my_creat(1, &path);
            ino = getino(wd, path);
        }
        else
        {
            printf("%s doesn't exist\n", path);
            return -1;
        }
    }

    mip = iget(wd->fs, ino);

    if (S_ISDIR(mip->INODE.i_mode))
    {
        printf("%s is a dir, cannot be opened\n", path);
        return -2;  // Special DIR return value
    }
    else if (!S_ISREG(mip->INODE.i_mode))
    {
        printf("%s is not a file, cannot be opened\n", path);
        return -1;
    }
    
    oget(mip, mode, &fd);

    time_t now = time(0L);
    mip->INODE.i_atime = now;
    if (mode != READ)
    {
        mip->INODE.i_mtime = now;
    }
    mip->dirty = 1;

    return fd;
}

int my_close(int fd)
{
    if (running->fd[fd] == NULL || fd < 0 || fd >= NFD)
    {
        printf("Invalid fd!\n");
    }

    OFT* op = running->fd[fd];

    if (running->fd[fd] != NULL)
    {
        oput(op);
        running->fd[fd] = NULL;
    }
    else
    {
        printf("fd %d already closed\n", fd);
    }

    return 0;
}

int my_lseek(int fd, int position)
{
    OFT* oftp = running->fd[fd];
    int original = oftp->offset;
    if(position > oftp->mptr->INODE.i_size)
    {
        oftp->offset = position;
    }
    return original;
}

int pfd()
{
    printf(" fd    mode    offset    INODE\n");
    printf("----   ----     ----     ------\n");
    for(int i = 0; i < 10; i++)
    {
        printf("%d    %s    %d    [%d, %d]\n", i, running->fd[i]->mode == 0? "READ" : running->fd[i]->mode == 1? "WRITE" : running->fd[i]->mode == 2? "READWRITE" : running->fd[i]->mode == 3? "APPEND": " ", running->fd[i]->offset, running->fd[i]->mptr->dev,running->fd[i]->mptr->ino);
    }

    return 0;
}
