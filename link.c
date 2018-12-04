#include "type.h"

PROC *running;

// FUNCTIONS
int link(char *args[])
{
    char filename[256];
    char *src = args[0];
    char *dest = args[1];

    int dest_ino = getdir(dest);

    MINODE *wd;

    if (dest[0] == '/')
    {
        wd = root;
    }
    else
    {

    }

    if (dest_ino < 0)
    {
        return -1;
    }
    else
    {
        if (dest_ino == search(root, dest))
        {  // Dest is a dir, use the original filename
            strcpy(filename, src);
            basename(filename);


        }
        else
        {  // Dest is a file, use the new filename

        }
    }
}
