/************** util.c file ****************/
#include "type.h"

char *tokens[64];
int dev;
int inode_start;

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

MINODE *iget(int dev, int ino)
{
    MINODE * mip;
    char buf[BLKSIZE];
    int i;
    INODE_LOCATION location;

    /*(1). Search minode[ ] for an existing entry (refCount > 0) with
       the needed (dev, ino):
       if found: inc its refCount by 1;
            return pointer to this minode;*/
    for (i = 0; i < NMINODE; i++)
    {
        if(minode[i].dev == dev && minode[i].ino == ino)
        {
            minode[i].refCount++;
            return &minode[i];
        }
    }

    /*(2). // needed entry not in memory:
       find a FREE minode (refCount = 0); Let mip-> to this minode;
       set its refCount = 1;
       set its dev, ino*/
    for (i = 0; i < NMINODE; i++)
    {
        if(minode[i].refCount == 0)
        {
            minode[i].refCount = 1;
            mip = &minode[i];
            mip->dev = dev;
            mip->ino = ino;
            break;
        }
    }

    //*(3). load INODE of (dev, ino) into mip->INODE:
    location = mailman(ino);

    get_block(dev, location.block, buf);
    ip = (INODE *) buf + location.offset;
    mip->INODE = *ip;  // copy INODE to mp->INODE

    return mip;
}

int iput(MINODE *mip) // dispose a used minode by mip
{
    char buf[BLKSIZE];
    INODE_LOCATION location;

    mip->refCount--;

    if (mip->refCount > 0) return 0;
    if (!mip->dirty)       return 0;

    location = mailman(mip->ino);

    get_block(dev, location.block, buf);
    
    INODE *ip = (INODE*) buf + location.offset;
    memcpy(ip, &mip->INODE, sizeof(INODE));

    put_block(dev, location.block, buf);

    return 0;
}

int search(INODE *ip, char *name)
{
    // Returns ino of name if it exists in ip
    char buf[BLKSIZE];
    char dirname[EXT2_NAME_LEN];

    char* cp;
    DIR * dp;

    for(int i = 0; i < 12; i++)
    {
        if(!ip->i_block[i])
        {
            // printf("No more blocks! %s not found!\n", name);
            break;
        }

        get_block(dev, ip->i_block[i], buf);
        dp = (DIR *) buf;
        cp = buf;

        while(cp < buf + BLKSIZE)
        {
            strncpy(dirname, dp->name, dp->name_len);
            dirname[dp->name_len] = '\0';

            if(!strcmp(dirname, name))
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
    char buf[BLKSIZE];

    int ino;
    int n = tokenize(pathname, "/", tokens);

    INODE* ip = &mip->INODE;

    for (int i = 0; i < n; i++)
    {
        ino = search(ip, tokens[i]);
        if (!ino)
        {
            // printf("can't find %s\n", tokens[i]);
            ino = -1;
            break;
        }
        ip = &iget(mip->dev, ino)->INODE;
    }
    return ino;
}

INODE_LOCATION mailman(int ino)
{
    INODE_LOCATION location;
    location.block  = (ino - 1) / 8 + inode_start;
    location.offset = (ino - 1) % 8;
    return location;
}

int getdir(INODE *ip, char *pathname)
{
    // Tries to get a valid directory from pathname
    char parent_path[256];
    int dest_ino;

    strcpy(parent_path, pathname);

    dest_ino = search(ip, pathname);

    if (dest_ino < 0)
    {
        // Try the parent, maybe the file doesn't exist yet
        strcpy(parent_path, dirname(parent_path));
        dest_ino = search(ip, parent_path);
    }

    if (dest_ino < 0)
    {
        printf("%s does not exist\n", parent_path);
        return -1;
    }
    else
    {
        // File exists: Check if it's a file or a dir
        MINODE *mip = iget(dev, dest_ino);
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

        get_block(dev, blk, buf);
  
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

            put_block(fd, blk, buf);
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

int ideal_len(DIR* dirent)
{
    return 4 * ((8 + dirent->name_len + 3) / 4);
}
