#include "type.h"

char buf[BLKSIZE];
FS     filesystems[NMOUNT], *root_fs, *cur_fs;

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
        wd = root_fs->root;
        dest++;
    }
    else
    {
        wd = running->cwd;
    }
    
    // Get INO of destination folder
    dest_ino = getdir(wd, dest);

    if (dest_ino < 0)
    {  // No valid destination directory
        return -1;
    }
    else
    {  // Get the filename for the destination directory
        if (dest_ino == search(wd, dest))
        {  // Dest is a dir, use the original filename
            strcpy(filename, src);
        }
        else
        {  // Dest is a file, use the new filename
            strcpy(filename, dest);
        }
        strcpy(filename, basename(filename));
    }

    if (dest[0] == '/')
    {
        wd = root_fs->root;
        dest++;
    }
    else
    {
        wd = running->cwd;
    }

    // Get INO of file to link
    src_ino = getino(wd, src);

    MINODE *to_link = iget(wd->fs, src_ino);
    MINODE *dir = iget(wd->fs, dest_ino);

    // Add the link to the directory
    if (!S_ISDIR(to_link->INODE.i_mode))
    {
        DIR entry;
        entry.inode = to_link->ino;
        entry.name_len = strlen(filename);
        strcpy(entry.name, filename);
        entry.rec_len = ideal_len(&entry);

        insert_entry(dir, &entry);

        iput(to_link);
        iput(dir);

        // Update the refCount in memory
        INODE_LOCATION location = mailman(to_link->fs, to_link->ino);

        get_block(wd->fs->dev, location.block, buf);
        INODE *link = (INODE *) buf + location.offset;
        link->i_links_count++;
        put_block(wd->fs->dev, location.block, buf);
        
        return 0;
    }
    else
    {
        printf("Can't create link to dir\n");
        return 1;
    }
}
