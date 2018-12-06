#include "type.h"
OFT oft[NOFT];

int my_open(int argc, char* args[])
{
    if(argc < 2)
    {
        printf("usage: open pathname mode\n");
        return 0;
    }
    char* path = args[0];
    char* modeStr = args[1];
    MINODE* wd;
    if (path[0] == '/')
    {
        // absolute path
        wd = root;
        dev = root->dev;
        path++;
    }
    else
    {
        ///relative path
        wd =running->cwd;
        dev = running->cwd->dev;
    }

    ///DETERMINE MODE WE ARE OPENING IN
    int mode = 0;
    if(strcmp(modeStr, "R") == 0 || strcmp(modeStr, "0") == 0)
    {
        mode = 0;
    }
    else if(strcmp(modeStr, "W") == 0 || strcmp(modeStr, "1") == 0)
    {
        mode = 1;
    }
    else if(strcmp(modeStr, "RW") == 0 || strcmp(modeStr, "2") == 0)
    {
        mode = 2;
    }
    else if(strcmp(modeStr, "APPEND") == 0 || strcmp(modeStr, "3") == 0)
    {
        mode = 3;
    }
    else
    {
        printf("Invalid mode! Cannot open\n");
        return 0;
    }

    int ino = getino(wd, path);

    if(ino == -1)
    {
        //file doesn't exist
        my_creat(args);
        ino = getino(wd, path);
    }

    MINODE* mip = iget(dev, ino);
    if(!S_ISREG(mip->INODE.i_mode))
    {
        printf("Not a regular file. Cannot be opened\n");
    }
 
    //  Check whether the file is ALREADY opened with INCOMPATIBLE mode:
    //        If it's already opened for W, RW, APPEND : reject.
    //        (that is, only multiple R are OK)

    for(int i = 0; i < NOFT; i++)
    {
        if(oft[i].mptr == mip && oft[i].mode != mode)
        {
            printf("File already opened in another mode\n");
            return 0;
        }
    }

    OFT* oftp;
    oftp->mode = mode;
    oftp->refCount = 1;
    oftp->mptr = mip;
    for(int i = 0; i < NOFT; i++)
    {
        if(oft[i].refCount == 0)
        {
            oft[i] = *oftp;
            break;
        }
    }

    switch(mode)
    {
        /// R and RW
        case 0: 
        case 2:
            oftp->offset = 0;
            break;
        /// W
        case 1: 
            truncate(mip);
            oftp->offset = 0;
            break;
        /// APPEND
        case 3:
            oftp->offset =mip->INODE.i_size;
            break;
        default: 
            printf("Invalid mode\n");
            return 0;
    }

    int i;
    for( i = 0; i < NFD; i++)
    {
        if(running->fd[i]  == NULL)
        {
            running->fd[i] = oftp;
            break;
        }
    }

    time_t now = time(0L);
    mip->INODE.i_atime = now;
    if(mode != 0)
    {
        mip->INODE.i_mtime = now;
    }
    mip->dirty = 1;

    return i;
}

int my_close(int fd)
{
    if(running->fd[fd] == NULL || fd < 0 || fd >= 10)
    {
        printf("Invalid fd!\n");
    }

    OFT* oftp;
    oftp = running->fd[fd];

    if(running->fd[fd] != 0)
    {
        oftp->refCount--;
        if(oftp->refCount == 0)
        {
            iput(oftp->mptr);
        }
    }
    running->fd[fd] = 0;
}

int lseek(int fd, int position)
{
    OFT* oftp = &running->fd[fd];
    int original = oftp->offset;
    if(position > oftp->mptr->INODE.i_size)
    {
        oftp->offset = position;
    }
    return original;
}

int pfd()