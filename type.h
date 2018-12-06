#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>

// TODO: Remove these later
#include <unistd.h>
#include <sys/stat.h>

/*************** CONST *********************************/
#define BLKSIZE  1024

#define NMINODE    64
#define NOFT       64
#define NFD        10
#define NMOUNT      4
#define NPROC       2

/*************** TYPES *********************************/
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

typedef struct inode_location{
    int block;
    int offset;
}INODE_LOCATION;

struct minode;
struct fs;

typedef struct minode{
    INODE INODE;
    int dev, ino;
    int refCount;
    int dirty;
    int mounted;
    struct fs *fs;
}MINODE;

typedef struct fs{
    struct minode minode[NMINODE];  // MINODEs currently allocated in memory
    struct minode *root;            // Root directory

    int dev;                 // Dev where FS is located
    int nblocks, ninodes;
    int bmap, imap;
    int inode_start;         // inode start block
}FS;

typedef struct oft{
    int  mode;
    int  refCount;
    MINODE *mptr;
    int  offset;
}OFT;

typedef struct proc{
    struct proc *next;  // Link to next proc... linked list behavior
    int          pid;
    int          uid, gid;
    MINODE      *cwd;
    OFT         *fd[NFD];
}PROC;

/*************** GLOBALS *********************************/
extern char *cmd_args[64], *tokens[64];  // Used by interpreter

extern PROC   proc[NPROC], *running;
extern FS     filesystems[NMOUNT], *root_fs, *cur_fs;

/*************** FUNCTIONS *********************************/
// UTIL.C
int get_block(int dev, int blk, char buf[]);
int put_block(int dev, int blk, char buf[]);
int tokenize(char *pathname, char *delim, char **tokens);
MINODE *iget(FS *fs, int ino);
int iput(MINODE *mip);
int getino(MINODE *mip, char *pathname);
int search(MINODE *mip, char *name);
INODE_LOCATION mailman(FS *fs, int ino);
int getdir(MINODE* mip, char *pathname);
int insert_entry(MINODE *dir, DIR *file);
int delete_entry(MINODE *dir, char *name);
int ideal_len(DIR *dirent);
int getlink(MINODE *mip, char buf[]);  // In symlink.c
int truncate(MINODE *mip);
char print_mode(u16 mode);

// ALLOC
int ialloc(FS *fs);
int balloc(FS *fs);
void idalloc(FS *fs, int ino);
void bdalloc(FS *fs, int bno);

// COMMANDS
int pwd(char *args[]);
int cd(char *args[]);
int quit(char *args[]);
int ls(char *args[]);
int my_link(char *args[]);
int my_symlink(char *args[]);
int my_unlink(char *args[]);
int my_mkdir(char *args[]);
int my_rmdir(char *args[]);
int my_creat(char *args[]);
int my_readlink(char *args[]);
int my_rmdir(char *args[]);
int my_stat(char *args[]);
int my_touch(char *args[]);
int my_chmod(char *args[]);
int my_menu(char* args[]);
