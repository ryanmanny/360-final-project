#include "type.h"

MINODE minode[NMINODE];

int quit(char *args[])
{
    for (int i = 0; i < NMINODE; i++)
    {
        if (minode[i].refCount > 0 && minode[i].dirty)
        {
            iput(&minode[i]);
        }
    }
    exit(0);
}
