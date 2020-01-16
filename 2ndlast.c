
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define DISKFILE  argv[1]

#define K   1024

typedef int           __le32;
typedef short int     __le16;
typedef unsigned char __u8;

struct ext2_super_block {
	__le32	s_inodes_count;		/* Inodes count */
	__le32	s_blocks_count;		/* Blocks count */
	__le32	s_r_blocks_count;	/* Reserved blocks count */
	__le32	s_free_blocks_count;	/* Free blocks count */
	__le32	s_free_inodes_count;	/* Free inodes count */
	__le32	s_first_data_block;	/* First Data Block */
	__le32	s_log_block_size;	/* Block size */
	__le32	s_log_frag_size;	/* Fragment size */
	__le32	s_blocks_per_group;	/* # Blocks per group */
	__le32	s_frags_per_group;	/* # Fragments per group */
	__le32	s_inodes_per_group;	/* # Inodes per group */
	__le32	s_mtime;		/* Mount time */
	__le32	s_wtime;		/* Write time */
	__le16	s_mnt_count;		/* Mount count */
	__le16	s_max_mnt_count;	/* Maximal mount count */
	__le16	s_magic;		/* Magic signature */
	__le16	s_state;		/* File system state */
	__le16	s_errors;		/* Behaviour when detecting errors */
	__le16	s_minor_rev_level; 	/* minor revision level */
	__le32	s_lastcheck;		/* time of last check */
	__le32	s_checkinterval;	/* max. time between checks */
	__le32	s_creator_os;		/* OS */
	__le32	s_rev_level;		/* Revision level */
	__le16	s_def_resuid;		/* Default uid for reserved blocks */
	__le16	s_def_resgid;		/* Default gid for reserved blocks */
	__le32	s_first_ino; 		/* First non-reserved inode */
	__le16  s_inode_size; 		/* size of each inode structure */
	__le16	s_block_group_nr; 	/* block group # of this superblock */
};

struct ext2_group_desc
{
        __le32  bg_block_bitmap;                /* Blocks bitmap block */
        __le32  bg_inode_bitmap;                /* Inodes bitmap block */
        __le32  bg_inode_table;         /* Inodes table block */
        __le16  bg_free_blocks_count;   /* Free blocks count */
        __le16  bg_free_inodes_count;   /* Free inodes count */
        __le16  bg_used_dirs_count;     /* Directories count */
        __le16  bg_pad;
        __le32  bg_reserved[3];
};
struct ext2_inode {
        __le16  i_mode;         /* File mode */
        __le16  i_uid;          /* Low 16 bits of Owner Uid */
        __le32  i_size;         /* Size in bytes */
        __le32  i_atime;        /* Access time */
        __le32  i_ctime;        /* Creation time */
        __le32  i_mtime;        /* Modification time */
        __le32  i_dtime;        /* Deletion Time */
        __le16  i_gid;          /* Low 16 bits of Group Id */
        __le16  i_links_count;  /* Links count */
        __le32  i_blocks;       /* Blocks count */
        __le32  i_flags;        /* File flags */
        __le32  unused;
        __le32  i_block[15];/* Pointers to blocks */
};

struct ext2_dir_entry {
        __le32  inode;                  /* Inode number */
        __le16  rec_len;                /* Directory entry length */
        __le16  name_len;               /* Name length */
        char    name[];                 /* File name, up to EXT2_NAME_LEN */
};

struct ext2_dir_entry_2 {
        __le32  inode;                  /* Inode number */
        __le16  rec_len;                /* Directory entry length */
        __u8    name_len;               /* Name length */
        __u8    file_type;
        char    name[];                 /* File name, up to EXT2_NAME_LEN */
};

char *typename[]={
        "UNKNOWN",
        "REG_FILE",
        "DIR",
        "CHRDEV",
        "BLKDEV",
        "FIFO",
        "SOCK",
        "SYMLINK",
};

int readsuper(int fd, struct ext2_super_block *sbptr);
int readgrouptab(int fd, struct ext2_group_desc *gdtptr, int blockno);
int readblock(int fd, __u8 b[K], int blockno);
char** split(char *path);
void freelist(char ** components);
int searchdir(int inopt, char * comp);
void printfile(int inoN);
//void printdir();
int inosize;
int inotable;
__u8 block[K];
int fd;
int pcount= 0;        // count number of path steps in input path
int pthino= 1;       //keep track of how much path is found or located
int flg = -1;        //status of search function

struct ext2_super_block *sb;      // superblock
struct ext2_group_desc *gdt;      // group descriptor table
struct ext2_inode *ino;           // inode
struct ext2_dir_entry_2 *dentry;  // directory entry

int main(int argc, char *argv[])
{


  //int fd;
  //int inosize;                      // superblock values needed
  //int inotable;                     // group descriptor table values needed
  int itemsize;
  int rootsize;
  int rootblock0;
  int rootblock1;
  int bitnumber;
  int blocknum;

  char *name;
  int bytenumber;
  int i,j,k,n;
  char ** t;
  char* path;

// open disk imsage
  fd = open(DISKFILE, O_RDONLY);

// read  superblock
  readblock(fd, block, 1); 
  sb = (struct ext2_super_block *) block;

// remember inode size
  inosize  = sb->s_inode_size;
  fprintf(stderr,"inode size: %d\n", inosize);


//
//read group descriptor table
  readblock(fd, block, 2);
  gdt = (struct ext2_group_desc *) block;

// remember inode size
  inotable = gdt-> bg_inode_table;          /* Inodes table block */
  fprintf(stderr,"inode table block: %d\n", inotable);

  



//search function !
  printf("Total arg values = %d \n", argc);
  for (int j =2; j < argc; j++ ){
  path = argv[j];
  
  t = split(path);
  pcount =0; 
  for (int i =0; t[i]; i++ ){
      pcount++;
  }
  //printf("total path steps = %d\n",pcount );

  pthino =1;  
  for (int i =0; i < pcount; i++){
        //if(t[i] != NULL)
        searchdir( pthino, t[i]); // return 0 for dir match
                                  // return 1 for file match
                                  // return 3 for no match
        if(flg== 0){
        //printf("Dir Match \n");
        }
        if(flg== 1){
        printf("file match\n");
        //printfile();

        }
        if((flg== 3)|| (pthino == 0) ){
        fprintf(stderr,"%s not found\n", path);
        break;
        }
        flg = -1;
  }
  printf("Done !!!!!!!!!!!!!!!! %s\n\n\n",argv[j]);
  }

/*
  // read inode table
  readblock(fd, block, inotable);   //reading first inode
  ino = (struct ext2_inode *) (block + inosize);     // root dir is at inode 1 in ext2 filesystems

// remember first block of root dir data
  rootsize    = ino->i_size;
  rootblock0  = ino->i_block[0];
  rootblock1  = ino->i_block[1];

  fprintf(stderr,"size: %d  block[0]: %d  block[1]: %d\n", rootsize, rootblock0, rootblock1);

// read first block of root directory data
  readblock(fd, block, rootblock0);
  bytenumber = 0;
  while(bytenumber < K ){
    dentry = (struct ext2_dir_entry_2 *) (block + bytenumber);
    name = block + bytenumber + sizeof(dentry);
    *(name + dentry->name_len) = 0;                            // paranoia
    bytenumber += dentry->rec_len;
    printf("%-12s %-10s %10d %6d %8d\n",
     name,
     typename[dentry->file_type],
     dentry->inode,
     dentry->rec_len,
     dentry->name_len);
  }
  
  printf("Reading next inode\n");
  
  // read inode table
  blocknum = (inosize * (15 -2) )/ K;
  bitnumber = (inosize * (15 -2)) % K;
  readblock(fd, block, inotable+blocknum);   //reading first inode
  printf("Reading done ! \n");
  
  ino = (struct ext2_inode *) (block + (inosize+ bitnumber));     // root dir is at inode 1 in ext2 filesystems

// remember first block of root dir data
  rootsize    = ino->i_size;
  rootblock0  = ino->i_block[0];
  rootblock1  = ino->i_block[1];

  fprintf(stderr,"size: %d  block[0]: %u  block[1]: %d\n", rootsize, rootblock0, rootblock1);

// read first block of root directory data
 
  readblock(fd, block, rootblock0);
  bytenumber = 0;
  while(bytenumber < K ){
    dentry = (struct ext2_dir_entry_2 *) (block + bytenumber);
    name = block + bytenumber + sizeof(dentry);
    *(name + dentry->name_len) = 0;                            // paranoia
    bytenumber += dentry->rec_len;
    printf("%-12s %-10s %10d %6d %8d\n",
     name,
     typename[dentry->file_type],
     dentry->inode,
     dentry->rec_len,
     dentry->name_len);
  }

*/







  /*
                        prints Split path !!!!!
  t = split(argv[2]);
  for (int i=0; t[i]; i++){
      printf("%d : %s\n", i, t[i]);
  }*/
  
  //searchdir( , t );

}

int searchdir(int inopt, char * comp){
  
  int rootsize;
  int rootblock;
  int rootblock1;
  int rootblock0;
  int bytenumber;
  int blocknum;
  int bitnum;
  char * name;
  int n;
    


  if(inopt == 1){
  // read inode table
  readblock(fd, block, inotable);   //reading first inode
  ino = (struct ext2_inode *) (block + inosize);     // root dir is at inode 1 in ext2 filesystems
}
 else{
     
     lseek(fd, (K * inotable)+(inopt-1 )*inosize , SEEK_SET);    // ino count starts from 0 but root inode is inode '1' 
     n = read(fd, block, K);
     ino = (struct ext2_inode *) (block); //reading from block buffer the size of ext2_inode data struct
     

     /*
  // read inode table
  blocknum = (inosize * (inopt -2) )/ K;
  bitnum = (inosize * (inopt -2)) % K;
  readblock(fd, block, inotable+blocknum);   //reading first inode
  
  ino = (struct ext2_inode *) (block + (inosize+ bitnum));     // root dir is at inode 1 in ext2 filesystems
 */
  }

// remember first block of root dir data
  rootsize    = ino->i_size;
  rootblock0  = ino->i_block[0];
  rootblock1  = ino->i_block[1];

  fprintf(stderr,"size: %d  block[0]: %d  block[1]: %d\n", rootsize, rootblock0, rootblock1);

// read first block of root directory data
  readblock(fd, block, rootblock0);
  bytenumber = 0;
  while(bytenumber < K ){
    dentry = (struct ext2_dir_entry_2 *) (block + bytenumber);
    name = block + bytenumber + sizeof(dentry);
    *(name + dentry->name_len) = 0;                            // paranoia
    bytenumber += dentry->rec_len;
    printf("%-12s %-10s %10d %6d %8d\n",        //printing dir
     name,
     typename[dentry->file_type],
     dentry->inode,
     dentry->rec_len,
     dentry->name_len);
    if((strcmp(name, comp) == 0) && (strcmp(typename[dentry -> file_type],"DIR") == 0 )) {         //comparing for required file/dir
        printf("%s MAtch !!!!inode is = %d \n", comp, dentry -> inode);
        flg = 0;
        pthino = dentry -> inode;
    }
    if((strcmp(name, comp) == 0) && (strcmp(typename[dentry -> file_type],"REG_FILE") == 0 )) {
         printf("%s file Match!!!\n", comp);
         

         flg = 1;
         pthino = dentry -> inode; // saving matched file inode number
         printfile( dentry -> inode );
         break;
        

    }   

  }
  if (flg == -1)
    flg = 3;


}

void printfile(int inoN){
    
    int n;
    int rootsize;
    int rootblock0;
    int rootblock1;
    __u8 blk [102400];
    int ceil;
    
    printf("reading inode no = %d\n", inoN);
    lseek(fd, (inotable* K) + ( (inoN -1)*inosize  )  ,SEEK_SET );
    n  = read(fd, block, K );
    ino = (struct ext2_inode *) (block);
    rootsize    = ino->i_size;
    rootblock0  = ino->i_block[0];
    rootblock1  = ino->i_block[1];

    fprintf(stderr,"size: %d  block[0]: %d  block[1]: %d\n", rootsize, rootblock0, rootblock1);
    
    
    // read next block[] ;
    printf("printing");
    ceil = (rootsize / K)+ ((rootsize % K) != 0);
    printf("\nceil of file size is %d * K\n", ceil);
    for (int i =0; i < (ceil); i++){

    readblock(fd,block, ino -> i_block[i]);
    //readblock(fd, blk, rootblock0);
    
    fprintf(stderr,"bytes read from ino block = %d\n", n);
    
    fprintf(stderr, "%s ", block);
    fprintf(stderr, "\n ---------------------------------------------- \n");
    
    }   

}


int readblock(int fd, __u8 b[K], int blockno)
{
  int n;

  lseek(fd, K * blockno, SEEK_SET);
  n = read(fd, b, K);
  if(n == K)
    return 0;
  fprintf(stderr,"only found %d bytes reading block %d\n", n, blockno);
  exit(1);
}



