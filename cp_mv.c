#include "type.h"

char *tokens[64];
FS     filesystems[NMOUNT], *root_fs, *cur_fs;

int my_cp(int argv,char* args[])
{
    char* src = args[0];
    char* dest = args[1];

    int fd = my_open(src, "R");
    int gd = my_open(dest, "RW");

    if (fd > 0 && fd > 0)
    {
        int n = 0;
        char buf[BLKSIZE];

        while((n = my_read(fd, buf, BLKSIZE)))
        {
            my_write(gd, buf, n);
        }

        my_close(fd);
        my_close(gd);

        return 0;
    }
    return 1;
}

int my_mv(int argv, char* args[])
{
    char* src = args[0];
    // char* dest = args[1]; 

    MINODE *wd;
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

    if (my_cp(2, args) == 0)  // my_cp succeeded
    {
        my_unlink(1, &src);
        return 0;
    }
    return 1;
}
