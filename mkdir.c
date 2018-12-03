#include "type.h"


MINODE minode[NMINODE];
MINODE *root;

PROC   proc[NPROC], *running;

char gpath[128];   // hold tokenized strings
char *name[64];    // token string pointers
int  n;            // number of token strings 

int  fd, dev;
int  nblocks, ninodes, bmap, imap, inode_start;
char line[128], cmd[32], pathname[64];

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
    char * parent = dirname(parentPath);
    char* child = basename(childPath);

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
    mymkdir(pip, child);
    pip->INODE.i_links_count++;
    
    //touch its atime and mark it DIRTY
    pip->dirty = 1;
    iput(pip);
}

int mymkdir(MINODE *pip, char *name)
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
}