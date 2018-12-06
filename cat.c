#include "type.h"

FS     filesystems[NMOUNT], *root_fs, *cur_fs;

int my_cat(int argc, char* args[])
{
    char *path = args[0];

    char buf[BLKSIZE];

    if (argc < 1)
    {
        puts("Usage: file");
        return 1;
    }

    int n = 0;
    int fd = my_open(path, "R");

    while ((n  = my_read(fd, buf, BLKSIZE)))
    {
        write(1, buf, n);
    }
    puts("");

    my_close(fd);

    return 0;
}
