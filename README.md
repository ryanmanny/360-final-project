# 360 Final Project
https://www.eecs.wsu.edu/~cs360/demoRecord.html

# Checklist
Team Members: Ryan Manny and Adolfo Garcia

============================= LEVEL 1 =========================================

Start with an empty disk for level-1

- startup: mount_root ✓
- ls ✓
- pwd ✓
- cd ✓
- mkdir ✓
- rmdir ✓
- creat ✓
- link ✓
- symlink ✓
- readlink ✓
- unlink ✓
- chmod ✓
- touch ✓
- stat ✓

============================ LEVEL 2 =========================================

Use samples/PROJECT/mydisk for level-2

- cat 
- cp
- mv

-------------------------------------------------------------------------------
    TEST:
    ls /X; cd /X; mkdir thisIsAnewdir; ls;

    cat /X/tiny 
    cat /Y/bigfile 
    cat /Z/hugefile

    cp  /Y/bigfile newbig;     ls;   cat newbig
    cp  /Z/hugefile newhuge;   ls;   cat newhuge

    NOTE: if can't do cat, cp: show individual commands of 
    lseek
    open
    write
    read
    close

============================ LEVEL 3 ==========================================

- mount
- umount
- permissions

-------------------------------------------------------------------------------
    TEST:
    mkdir /mnt
    mount FS /mnt;  mount (show what's mounted)

    cd to mounted FS; mkdir newdir; creat newfile; ls
    cd back to /; umount FS
    check FS under Linux: 
    mount -o loop FS /mnt; ls /mnt 
    (newdir and newfile MUST still exist)

    permission  : try to rmdir dir AND unlink file NOT belong to proc:
