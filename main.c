#include "type.h"

char *cmd_args[64];  // Used by interpreter
char line[256], command[128], cmd_arg_str[128];

PROC   proc[NPROC], *running;
FS     filesystems[NMOUNT], *root_fs, *cur_fs;

// FUNCTION POINTER TABLE
char *cmd_strs[] = {
    "ls",
    "pwd",
    "cd",
    "mkdir",
    "rmdir",
    "creat",
    "link",
    "symlink",
    "readlink",
    "unlink",
    "chmod",
    "touch",
    "stat",
    // "cat",
    // "cp",
    // "mv",
    // "mount",
    // "umount",
    "menu",
    "quit"
};

int (*cmds[])(char **) = {
    (int (*)()) 
    ls,
    pwd,
    cd,
    my_mkdir,
    my_rmdir,
    my_creat,
    my_link,
    my_symlink,
    my_readlink,
    my_unlink,
    my_chmod,
    my_touch,
    my_stat,
    // cat,
    // cp,
    // mv,
    // mount,
    // umount,
    my_menu,
    quit
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
    PROC   *p;

    printf("init()\n");

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

int mount_root(FS *fs, char *fs_name)
{
    MINODE *mip;

    printf("mount_root()\n");
    for (int i = 0; i < NMINODE; i++)
    {
        mip = &fs->minode[i];
        // set all entries to 0;
        mip->dev = 0;
        mip->ino = 0;
        mip->refCount = 0;
        mip->dirty = 0;
        mip->mounted = 0;
        mip->fs = NULL;
    }

    char buf[BLKSIZE];
    int dev;

    SUPER *sp;
    GD    *gp;
    
    // open device for RW (get a file descriptor as dev for the opened device)
    // TODO: Pass AS ARGUMENT
    dev = open(fs_name, O_RDWR);
    
    if (dev < 0)
    {
        printf("Cannot open %s!\n", fs_name);
        exit(0);
    }
    fs->dev = dev;

    // read SUPER block to verify it's an EXT2 FS
    get_block(fs->dev, 1, buf);  
    sp = (SUPER *) buf;

    // check for EXT2 magic number:
    if (sp->s_magic != 0xEF53)
    {
        printf("NOT an EXT2 FS\n");
        exit(1);
    }

    // record nblocks, ninodes as globals
    fs->nblocks = sp->s_blocks_count;
    fs->ninodes = sp->s_inodes_count;

    //read GD0; record bamp, imap, inodes_start as globals;

    get_block(dev, 2, buf);
    gp = (GD *) buf;

    fs->imap = gp->bg_inode_bitmap;
    fs->bmap = gp->bg_inode_bitmap;
    fs->inode_start = gp->bg_inode_table;
    
    fs->root = iget(fs, 2);    /* get root inode */

    // Let cwd of both P0 and P1 point at the root minode (refCount=3) ??? TODO
    proc[0].cwd = iget(fs, 2); 
    proc[1].cwd = iget(fs, 2);
    printf("root refCount = %d\n", fs->root->refCount);

    //Let running -> P0
    running = &proc[0];

    return 0;
}

int main(int argc, char *argv[])
{
    init();
    
    root_fs = &filesystems[0];
    if (argc < 2)
        mount_root(root_fs, "mydisk");
    else
        mount_root(root_fs, argv[1]);
    cur_fs = root_fs;

    char *temp;

    while (1)
    {
        printf("Enter a command:\n");  // TODO: Dynamically print valid commands
        fgets(line, 256, stdin);
        line[strlen(line) - 1] = '\0'; // kill \n at end of line
        
        strcpy(command, strtok(line, " "));
        temp = strtok(NULL, "\n");
        if (temp)
            strcpy(cmd_arg_str, temp);  // Cut of tail of command for args
        else
            strcpy(cmd_arg_str, "");

        run_command(command, cmd_arg_str);

        strcpy(line, "");  // Reset line for next part
    }

    return 1;  // Somehow got out of loop?
}
