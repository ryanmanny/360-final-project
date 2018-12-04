#include "type.h"


MINODE minode[NMINODE];
MINODE *root;

PROC   proc[NPROC], *running;

char gpath[128];   // hold tokenized strings
char *name[64];    // token string pointers
int  n;            // number of token strings 

int  fd, dev;
int  nblocks, ninodes, bmap, imap, inode_start;
char cmd[32], pathname[64];

int make_dir(char ** args)
{
    MINODE * mip;
    char* path = args[0];

    // path is pathname we wanna create
    if(path[0] == '/')
    {
        // absolute path
        mip = root; 
        dev = root->dev;
    }
    else{
        ///relative path
        mip = running->cwd;
        dev = running->cwd->dev;
    }   
    char parentPath[128], childPath[128];
    /// basename and dirname destroy original string, so gotta make new copies of them
    strcpy(parentPath, path);
    strcpy(childPath, path);
    char parent[BLKSIZE], child[BLKSIZE];
    strcpy(parent, dirname(parentPath));
    strcpy(child, basename(childPath));

    int pino;
    MINODE* pip;
    pino = getino(parent);
    pip = iget(dev, pino);
    char buf[BLKSIZE], *cp;
    int childFound = 0;

    //checking if parent INODE is a dir 
    if(S_ISDIR(pip->INODE.i_mode))
    {
        /// check child does not exist in parent directory
        get_block(dev, pip->INODE.i_block[0], buf);
        dp = (DIR *) buf;
        cp = buf;
        char dirname[EXT2_NAME_LEN];
        int childINo = getino(child);

        while(cp < buf +  1024)
        {
            strcpy(dirname, dp->name);
            dirname[dp->name_len] = '\0';
            if(dp->inode == childINo)
            {
                //child exists in directory
                childFound = 1;
                break;
            }
            cp += dp->rec_len;
            dp = (DIR*) cp;
        }
    }
    mymkdir(pip, child, pino);
    pip->INODE.i_links_count++;
    
    //touch its atime and mark it DIRTY
    pip->dirty = 1;
    iput(pip);
}

int mymkdir(MINODE *pip, char *name, int parentIno)
{
    int ino = ialloc(dev);
    int bno = balloc(dev);
     char buf[BLKSIZE], *cp;

    MINODE* mip = iget(dev, ino);
    INODE *ip = &mip->INODE;

    ip->i_mode = 0X41ED;
    ip->i_uid  = running->uid;	// Owner uid 
    ip->i_gid  = running->gid;	// Group Id
    ip->i_size = BLKSIZE;		// Size in bytes 
    ip->i_links_count = 2;	        // Links count=2 because of . and ..
    ip->i_mtime = time(0L);
    ip->i_ctime = ip->i_mtime;
    ip->i_atime = ip->i_ctime;   // set to current time
    ip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks 
    ip->i_block[0] = bno;             // new DIR has one data block   
    int i = 0;
    for(i = 1; i < 15; i++)
        ip->i_block[i] = 0;
    mip->dirty = 1;

    iput(mip);

    DIR* dp = (DIR*) buf;
    dp->inode = ino;
    dp->name_len = 1;
    dp->rec_len = 12;
    strcpy(dp->name, ".");
    cp += dp->rec_len;
    dp = (DIR*) cp;
    dp->inode = parentIno;
    dp->name_len = 2;
    dp->rec_len = 1012;
    strcpy(dp->name, "..");

    put_block(dev, bno, buf);
    enter_name(pip, ino, name);
}

int enter_name(MINODE* pip, int myino, char* myname)
{
    int i = 0;
    char buf[BLKSIZE], *cp;
    for(i = 0; i < 12; i ++ )
    {
        if(pip->INODE.i_block[i] == 0)
            break;

        // get parent's ith data block into a buf[ ] 
        get_block(dev, pip->INODE.i_block[i], buf);
  
        dp = (DIR *)buf;
        cp = buf;
        int need_length = 4 * ((8 + dp->name_len + 3) / 4);
        /// blk is last entry in block
        int blk = pip->INODE.i_block[i];

        printf("step to LAST entry in data block %d\n", blk);
        while (cp + dp->rec_len < buf + BLKSIZE){
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }

        int remain = dp->rec_len - need_length;
        if(remain >= need_length)
        {
            dp->rec_len = need_length;
            cp += dp->rec_len;
            dp = (DIR *)cp;
            dp->inode = myino;
            dp->name_len = strlen(myname);
            dp->rec_len = remain;
            strcpy(dp->name, myname);
            put_block(fd, blk, buf);
        }
        else
        {
            int bno = balloc(dev);
            pip->INODE.i_size += BLKSIZE;
            get_block(dev, bno, buf);
            dp = (DIR*) dp;
            cp= buf;
            dp->rec_len = need_length;
            dp->inode = myino;
            dp->name_len = strlen(myname);
            strcpy(dp->name, myname);
        }   
    }
}