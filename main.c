#include "type.h"

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
char *cmd_args[64];
int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start;
char line[256], command[128], cmd_arg_str[128];

// FUNCTION POINTER TABLE
char *cmd_strs[] = {
    "ls",
    "pwd",
    "cd",
    "mkdir",
    // "rmdir ",
    // "creat",
    "link"
    // "symlink",
    // "unlink",
    // "chmod",
    // "touch",
    // "stat",
    // "cat ",
    // "cp",
    // "mv",
    // "mount",
    // "umount"
};

int (*cmds[])(char **) = {
    (int (*)()) 
    ls,
    pwd,
    cd,
    my_mkdir,
    // rmdir ,
    // creat,
    my_link
    // symlink,
    // unlink,
    // chmod,
    // touch,
    // stat,
    // cat ,
    // cp,
    // mv,
    // mount,
    // umount
};

int run_command(char *cmd, char *args)
{
    int num_commands = sizeof(cmd_strs) / sizeof(char *);

    for (int i = 0; i < num_commands; i++)
    {
        if (!strcmp(cmd, cmd_strs[i]))
        {
            tokenize(args, " ", cmd_args);
            return cmds[i](cmd_args);
        }
    }
    puts("Invalid command!");
    return -1;
}

// FUNCTIONS
int init()
{
    int i;
    MINODE *mip;
    PROC   *p;

    printf("init()\n");

    root = NULL;

    for (i = 0; i < NMINODE; i++)
    {
        mip = &minode[i];
        // set all entries to 0;
        mip->dev = 0;
        mip->ino = 0;
        mip->refCount = 0;
        mip->dirty = 0;
        mip->mounted = 0;
        mip->mptr = 0;
    }
    for (i = 0; i < NPROC; i++)
    {
        p = &proc[i];
        // set pid = i; uid = i; cwd = 0;  TODO: Check if this is correct
        p->pid = i;
        p->uid = i;
        p->cwd = 0;
        p->next = 0;
        p->cwd = 0;
    }

    return 0;
}

int mount_root()
{
    printf("mount_root()\n");
    char buf[BLKSIZE];
    
    // open device for RW (get a file descriptor as dev for the opened device)
    // TODO: Pass AS ARGUMENT
    dev = open("mydisk", O_RDWR);
    fd = dev;
    printf("dev: %d\n",dev);
    if (dev < 0)
    {
        printf("Cannot open!\n");
        exit(0);
    }

    // read SUPER block to verify it's an EXT2 FS
    get_block(fd, 1, buf);  
    sp = (SUPER *)buf;

    // check for EXT2 magic number:
    if (sp->s_magic != 0xEF53)
    {
        printf("NOT an EXT2 FS\n");
        exit(1);
    }

    // record nblocks, ninodes as globals
    nblocks = sp->s_blocks_count;
    ninodes = sp->s_inodes_count;

    //read GD0; record bamp, imap, inodes_start as globals;

    get_block(dev, 2, buf);
    GD* gp = (GD *)buf;
    imap = gp->bg_inode_bitmap;
    bmap = gp->bg_inode_bitmap;
    inode_start = gp->bg_inode_table;
    
    root = iget(dev, 2);    /* get root inode */

    // Let cwd of both P0 and P1 point at the root minode (refCount=3) ??? TODO
    proc[0].cwd = iget(dev, 2); 
    proc[1].cwd = iget(dev, 2);
    printf("root refCount = %d\n", root->refCount);

    //Let running -> P0
    running = &proc[0];

    return 0;
}

int main()
{
    init();
    mount_root();

    char *temp;

    while(1)
    {
        printf("Enter a command:\n");  // TODO: Dynamically print valid commands
        fgets(line, 256, stdin);
        line[strlen(line) - 1] = '\0'; // kill \n at end of line
        
        strcpy(command, strtok(line, " "));
        temp = strtok(NULL, "\n");
        if (temp)
            strcpy(cmd_arg_str, temp);  // Tokenize the rest of the line to get args
        else
            strcpy(cmd_arg_str, "");

        run_command(command, cmd_arg_str);

        strcpy(line, "");  // Reset line for next part
    }

    return 1;  // Somehow got out of loop?
}
