#include "type.h"

char buf[BLKSIZE];
PROC *running;

// FUNCTIONS
int my_link(char *args[])
{
    char filename[256];
    int src_ino, dest_ino;
    char *src = args[0];
    char *dest = args[1];

    MINODE *wd;

    if (dest[0] == '/')
    {
        wd = root;
        dest++;
    }
    else
    {
        wd = running->cwd;
    }

    // Get INO of destination folder
    dest_ino = getdir(&wd->INODE, dest);

    if (dest_ino < 0)
    {  // No valid destination directory
        return -1;
    }
    else
    {  // Get the filename for the destination directory
        if (dest_ino == search(&wd->INODE, dest))
        {  // Dest is a dir, use the original filename
            strcpy(filename, src);
            basename(filename);
        }
        else
        {  // Dest is a file, use the new filename
            strcpy(filename, dest);
            basename(filename);
        }
    }

    if (src[0] == '/')
    {
        wd = root;
        src++;
    }
    else
    {
        wd = running->cwd;
    }

    // Get INO of file to link
    src_ino = search(&wd->INODE, src);

    MINODE *to_link = iget(dev, src_ino);
    MINODE *dir = iget(dev, dest_ino);

    // Add the link to the directory
    DIR entry;
    entry.inode = to_link->ino;
    entry.name_len = strlen(filename);
    strcpy(entry.name, filename);
    entry.rec_len = ideal_len(&entry);

    insert_entry(dir, &entry, filename);

    iput(to_link);
    iput(dir);

    // Update the refCount in memory
    INODE_LOCATION location = mailman(to_link->ino);

    get_block(dev, location.block, buf);
    INODE *link = (INODE *) buf + location.offset;
    link->i_links_count++;
    put_block(dev, location.block, buf);
}
