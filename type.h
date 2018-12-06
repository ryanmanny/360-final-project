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

extern OFT    oft[NOFT];
extern PROC   proc[NPROC], *running;
extern FS     filesystems[NMOUNT], *root_fs, *cur_fs;

/*************** FUNCTIONS *********************************/
// UTIL.C
int get_block(int dev, int blk, char buf[]);
int put_block(int dev, int blk, char buf[]);
int tokenize(char *pathname, char *delim, char **tokens);
MINODE *iget(FS *fs, int ino);
int iput(MINODE *mip);
OFT *oget(MINODE *mip, int mode, int *fd);
int oput(OFT *op);
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
int get_ith_block(MINODE *mip, int i);

// FILE MANIPULATION
int my_read(int fd, char buf[], int nbytes);
int my_write(int fd, char buf[], int nbytes);
int my_open(char* path, char* modeStr);
int my_close(int fd);
int my_lseek(int fd, int position);

// ALLOC
int ialloc(FS *fs);
int balloc(FS *fs);
void idalloc(FS *fs, int ino);
void bdalloc(FS *fs, int bno);

// COMMANDS

// LEVEL 1
int pwd(int argc, char *args[]);
int cd(int argc, char *args[]);
int quit(int argc, char *args[]);
int ls(int argc, char *args[]);
int my_link(int argc, char *args[]);
int my_symlink(int argc, char *args[]);
int my_unlink(int argc, char *args[]);
int my_mkdir(int argc, char *args[]);
int my_rmdir(int argc, char *args[]);
int my_creat(int argc, char *args[]);
int my_readlink(int argc, char *args[]);
int my_rmdir(int argc, char *args[]);
int my_stat(int argc, char *args[]);
int my_touch(int argc, char *args[]);
int my_chmod(int argc, char *args[]);
int my_menu(int argc, char *args[]);

// LEVEL 2
int my_cat(int argc, char *args[]);
int my_cp(int argc, char *args[]);
int my_mv(int argc, char *args[]);
