/************** util.c file ****************/
#include "type.h"

char *tokens[64];
FS     filesystems[NMOUNT], *root_fs, *cur_fs;

// FUNCTIONS
int get_block(int fd, int blk, char buf[ ])
{
    lseek(fd, (long) blk * BLKSIZE, 0);
    read(fd, buf, BLKSIZE);

    return 0;
}

int put_block(int fd, int blk, char buf[ ])
{
    lseek(fd, (long) blk * BLKSIZE, 0);
    write(fd, buf, BLKSIZE);

    return 0;
}

int tokenize(char *str, char *delim, char *tokens[])
{
    // Tokenize str into tokens array, assume it's large enough
    int count = 0;

    char *temp = strtok(str, delim);  // first call to strtok()
    if (!temp) return 0;
    else tokens[count++] = temp;

    // Call strtok() until it returns NULL
    while ((tokens[count] = strtok(NULL, delim)))
    {
        count++;
    }

    // Return # of tokens
    return count;
}

MINODE *iget(FS *fs, int ino)
{
    // Get a MINODE * matching ino from a filesystem
    MINODE *mip;
    INODE *ip;

    char buf[BLKSIZE];
    INODE_LOCATION location;

    /*(1). Search minode[] for an existing entry (refCount > 0) with
       the needed (dev, ino):
       if found: inc its refCount by 1;
            return pointer to this minode;*/
    for (int i = 0; i < NMINODE; i++)
    {
        if (fs->minode[i].dev == fs->dev && fs->minode[i].ino == ino)
        {
            fs->minode[i].refCount++;
            return &fs->minode[i];
        }
    }

    /*(2). // needed entry not in memory:
       find a FREE minode (refCount = 0); Let mip-> to this minode;
       set its refCount = 1;
       set its dev, ino*/
    for (int i = 0; i < NMINODE; i++)
    {
        if (fs->minode[i].refCount == 0)
        {
            fs->minode[i].refCount = 1;
            mip = &fs->minode[i];
            mip->dev = fs->dev;
            mip->ino = ino;
            mip->fs = fs;
            break;
        }
    }

    //*(3). load INODE of (dev, ino) into mip->INODE:
    location = mailman(fs, ino);

    get_block(fs->dev, location.block, buf);
    ip = (INODE *) buf + location.offset;
    mip->INODE = *ip;  // copy INODE to mp->INODE

    return mip;
}

int iput(MINODE *mip)
{
    char buf[BLKSIZE];
    INODE_LOCATION location;

    mip->refCount--;

    if (mip->refCount > 0) return 0;
    if (!mip->dirty)       return 0;

    location = mailman(mip->fs, mip->ino);

    get_block(mip->dev, location.block, buf);
    
    INODE *ip = (INODE*) buf + location.offset;
    memcpy(ip, &mip->INODE, sizeof(INODE));

    put_block(mip->dev, location.block, buf);

    return 0;
}

int search(MINODE *mip, char *name)
{
    // Returns ino of name if it exists in ip
    char buf[BLKSIZE];
    char dirname[EXT2_NAME_LEN];

    char* cp;
    DIR * dp;

    for (int i = 0; i < 12; i++)
    {
        if (!mip->INODE.i_block[i])
        {
            // printf("No more blocks! %s not found!\n", name);
            break;
        }

        get_block(mip->dev, mip->INODE.i_block[i], buf);
        dp = (DIR *) buf;
        cp = buf;

        while (cp < buf + BLKSIZE)
        {
            strncpy(dirname, dp->name, dp->name_len);
            dirname[dp->name_len] = '\0';

            if (!strcmp(dirname, name))
            {
                return dp->inode;
            }

            cp += dp->rec_len;
            dp = (DIR*) cp;
        }
    }

    // Couldn't find ino
    return -1;
}

int getino(MINODE *mip, char *pathname)
{
    // Return ino of pathname
    int ino;
    int n = tokenize(pathname, "/", tokens);

    for (int i = 0; i < n; i++)
    {
        ino = search(mip, tokens[i]);
        if (!ino)
        {
            // printf("can't find %s\n", tokens[i]);
            ino = -1;
            break;
        }
        mip = iget(mip->fs, ino);
    }
    return ino;
};

INODE_LOCATION mailman(FS *fs, int ino)
{
    INODE_LOCATION location;
    location.block  = (ino - 1) / 8 + fs->inode_start;
    location.offset = (ino - 1) % 8;
    return location;
}

int getdir(MINODE *mip, char *pathname)
{
    // Tries to get a valid directory from pathname
    char parent_path[256];
    int dest_ino;

    strcpy(parent_path, pathname);

    dest_ino = search(mip, pathname);

    if (dest_ino < 0)
    {
        // Try the parent, maybe the file doesn't exist yet
        strcpy(parent_path, dirname(parent_path));
        dest_ino = search(mip, parent_path);
    }

    if (dest_ino < 0)
    {
        printf("%s does not exist\n", parent_path);
        return -1;
    }
    else
    {
        // File exists: Check if it's a file or a dir
        mip = iget(mip->fs, dest_ino);
        if (!S_ISDIR(mip->INODE.i_mode))
        {
            printf("%s already exists\n", parent_path);
            return -1;
        }
        else
        {
            return dest_ino;
        }
    }
}

int insert_entry(MINODE *dir, DIR *file)
{
    // Insert dirent for file into dir's datablocks

    int blk, required, remain;
    char buf[BLKSIZE], *cp;

    DIR *dp, *last_rec, *new_rec;

    required = ideal_len(file);

    for (int i = 0; i < 12; i++)
    {
        blk = dir->INODE.i_block[i];

        if (!dir->INODE.i_block[i])
            break;

        get_block(dir->dev, blk, buf);
  
        dp = (DIR *) buf;  // Begin traversing data blocks
        cp = buf;
        
        // Find last entry in dir
        while (cp + dp->rec_len < buf + BLKSIZE)
        {
            cp += dp->rec_len;
            dp = (DIR *) cp;
        }
        last_rec = dp;

        // How many bytes remain in current block
        remain = last_rec->rec_len;

        if (remain >= required)
        {
            last_rec->rec_len = ideal_len(last_rec);
            remain -= last_rec->rec_len;
            
            cp += last_rec->rec_len;
            new_rec = (DIR *) cp;
            
            new_rec->inode = file->inode;
            new_rec->name_len = file->name_len;
            new_rec->rec_len = remain;
            strncpy(new_rec->name, file->name, file->name_len);

            put_block(dir->dev, blk, buf);
        }
        else
        {
            printf("Raise!!! we need to allocate another block");
            return 1;
        }
        // else
        // {
        //     int bno = balloc(dev);
        //     dir->INODE.i_size += BLKSIZE;
        //     get_block(dev, bno, buf);
        //     dp = (DIR*) dp;
        //     cp= buf;
        //     dp->rec_len = need_length;
        //     dp->inode = file->inode;
        //     dp->name_len = file->name_len;
        //     strcpy(dp->name, file->name);
        //     put_block(fd, ip->i_block[i + 1], buf);
        // }
    }

    return 0;
}

int delete_entry(MINODE* dir, char* name)
{
    char buf[BLKSIZE];

    DIR *dp, *prev = NULL;
    char *cp;

    DIR *delete = NULL, *before_delete = NULL, *last = NULL;

    int affected_block = -1;
    
    for (int i = 0; i < 12; i++)
    {
        if (!dir->INODE.i_block[i])
        {
            printf("No more blocks! %s not found!\n", name);
            return 1;
        }

        get_block(dir->dev, dir->INODE.i_block[i], buf);

        cp = buf;
        dp = (DIR *) buf;

        while (cp < buf + BLKSIZE)
        {
            if (!strncmp(dp->name, name, dp->name_len))
            {
                // We found the dir we wanted
                before_delete = prev;
                delete = dp;
                affected_block = i;  // This is the block that will be changed
            }

            prev = dp;
            cp += dp->rec_len;       // Move to next block
            dp = (DIR *) cp;
        }
        if (affected_block != -1)
        {
            last = prev;
            break;
        }
    }

    if (delete->rec_len == BLKSIZE)  // Only entry
    {
        bdalloc(dir->fs, dir->INODE.i_block[affected_block]);  // Boof the entire block
        
        dir->INODE.i_size -= BLKSIZE; // Decrement the block by the size of an entire block

        for (int j = affected_block; j < 11; j++)  // Scoot the next blocks over to the left by one
        {
            dir->INODE.i_block[j] =  dir->INODE.i_block[j + 1];
        }
        dir->INODE.i_block[11] = 0;
    }
    else
    {
        if (delete == last)  // Last entry in the block
        {
            // Increase size of previous entry to overwrite current dirent
            before_delete->rec_len += delete->rec_len;
        }
        else // This wasn't the last entry in the block
        {
            // Increase length of last entry by amount deleted
            last->rec_len += delete->rec_len;

            // Scoot the entire memoryspace over to squash the deleted entry
            memcpy((char *) delete, (char *) delete + delete->rec_len, (int) (buf + BLKSIZE - (char *) delete));
        }
    }
    
    put_block(dir->dev, dir->INODE.i_block[affected_block], buf);
    dir->dirty = 1;
    
    return 0;
}

int ideal_len(DIR* dirent)
{
    return 4 * ((8 + dirent->name_len + 3) / 4);
}

int truncate(MINODE *mip)
{
    // Deallocates all of the blocks used by inode
    int block, i = 0;;

    // While there are still blocks
    while ((block = get_ith_block(mip, i) != 0))
    {
        bdalloc(mip->fs, block);
        i++;
    }

    return i;  // Return number of blocks deallocated
}

char print_mode(u16 mode)
{
    char *mask  = "rwxrwxrwx";
    char *bmask = "---------";
    int index = 0;

    char filetype = '\0';

    if (S_ISDIR(mode))
        filetype = 'd';  // DIR
    else if (S_ISLNK(mode))
        filetype = 'l';  // LINK
    else
        filetype = ' ';  // REG

    printf("%c", filetype);

    for (int shift = 8; shift >= 0; shift--)
    {
        if ((mode >> shift) & 1)
        {
            printf("%c", mask[index]);
        }
        else
        {
            printf("%c", bmask[index]);
        }
        index++;
    }

    return filetype;
}

int get_ith_block(MINODE *mip, int i)
{
    char buf[BLKSIZE];
    // Get corresponding block # for index i in order
    INODE *ip = &mip->INODE;

    int n = BLKSIZE / sizeof(int);  // Number of bnos stored in each block
    
    int num_blocks = 12;
    int num_iblocks = n;
    int num_diblocks = n * n;

    if (i < 0)
    {
        puts("Invalid block number!");
        return -1;
    }
    // DIRECT BLOCKS: 0 <= i < 11
    else if (i < num_blocks)
    {
        return ip->i_block[i];
    }
    // INDIRECT BLOCK: 12
    else if (i < num_blocks + num_iblocks)
    {
        if (ip->i_block[12] != 0)
        {
            get_block(mip->dev, ip->i_block[12], buf);
            
            return ((int *) buf)[i - num_blocks];
        }
    }
    // DOUBLE INDIRECT BLOCK: 13
    else if (i < num_blocks + num_iblocks + num_diblocks)
    {
        if (ip->i_block[13] != 0)
        {
            // Get double indirect block contents
            get_block(mip->dev, ip->i_block[13], buf);

            int block  = ((i - (num_blocks + num_iblocks)) / n);
            int offset = ((i - (num_blocks + num_iblocks)) % n);
            
            // Get the corresponding 
            if (((int *) buf)[block] != 0)
            {
                get_block(mip->dev, ((int *) buf)[block], buf);

                return ((int *) buf)[offset];
            }
        }
    }
    return 0;
}
