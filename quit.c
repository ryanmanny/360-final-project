#include "type.h"

FS     filesystems[NMOUNT], *root_fs, *cur_fs;

int quit(int argc, char *args[])
{
    for (int n = 0; n < NMOUNT; n++)
    {
        FS *fs = &filesystems[n];
        for (int i = 0; i < NMINODE; i++)
        {
            if (fs->minode[i].refCount > 0 && fs->minode[i].dirty)
            {
                iput(&fs->minode[i]);
            }
        }
    }
    exit(0);
}
