#include "type.h"

char *tokens[64];
FS     filesystems[NMOUNT], *root_fs, *cur_fs;

int my_cp(int argv,char* args[])
{
    char* src = args[0];
    char* dest = args[1]; 
    int fd = my_open(src, "R");
    int gd = mu_open(dest, "RW");
    
    int n = 0;
    char* buf[BLKSIZE];
    while( n = read(fd, buf, BLKSIZE))
    {
        write(gd, buf, n);
    }

    return 0;
}

int my_mv(int argv,char* args[])
{
    char* src = args[0];
    char* dest = args[1]; 
    // path is pathname we wanna create

    MINODE * mip, *wd;
    if (src[0] == '/')
    {
        // absolute path
        wd = root_fs->root;
        src++;
    }
    else
    {
        ///relative path
        wd =running->cwd;
    }
    
    int ino = getino(wd, src);
    if(ino == -1)
    {
        printf("%s does not exist\n", src);
    }
    MINODE* mip = iget(wd->fs, ino);

    if(mip->dev == wd->dev)
    {
        
    }
}