#include <stdio.h>
#include <stdlib.h>
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

SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;   

typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;
  int dirty;
  int mounted;
  struct mntable *mptr;
}MINODE;

typedef struct inode_location{
  int block;
  int offset;
}INODE_LOCATION;

typedef struct oft{
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;

typedef struct proc{
  struct proc *next;
  int          pid;
  int          uid, gid;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;

/*************** GLOBALS *********************************/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern char *tokens[64], *cmd_args[64];
extern int n;
extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;
extern char line[256], command[128], cmd_arg_str[128];

/*************** FUNCTIONS *********************************/
// UTIL.C
int get_block(int fd, int blk, char buf[ ]);
int put_block(int fd, int blk, char buf[ ]);
int tokenize(char *pathname, char *delim, char **tokens);
MINODE *iget(int dev, int ino);
int iput(MINODE *mip);
int getino(char *pathname);
int search(INODE *ip, char *name);
INODE_LOCATION mailman(int ino);
int getdir(INODE* ip, char *pathname);

// COMMANDS
int pwd(char *args[]);
int cd(char *args[]);
int quit(char *args[]);
int ls(char *args[]);
