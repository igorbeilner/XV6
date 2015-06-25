3900 // On-disk file system format.
3901 // Both the kernel and user programs use this header file.
3902 
3903 // Block 0 is unused.
3904 // Block 1 is super block.
3905 // Blocks 2 through sb.ninodes/IPB hold inodes.
3906 // Then free bitmap blocks holding sb.size bits.
3907 // Then sb.nblocks data blocks.
3908 // Then sb.nlog log blocks.
3909 
3910 #define ROOTINO 1  // root i-number
3911 #define BSIZE 512  // block size
3912 
3913 // File system super block
3914 struct superblock {
3915   uint size;         // Size of file system image (blocks)
3916   uint nblocks;      // Number of data blocks
3917   uint ninodes;      // Number of inodes.
3918   uint nlog;         // Number of log blocks
3919 };
3920 
3921 #define NDIRECT 12
3922 #define NINDIRECT (BSIZE / sizeof(uint))
3923 #define MAXFILE (NDIRECT + NINDIRECT)
3924 
3925 // On-disk inode structure
3926 struct dinode {
3927   short type;           // File type
3928   short major;          // Major device number (T_DEV only)
3929   short minor;          // Minor device number (T_DEV only)
3930   short nlink;          // Number of links to inode in file system
3931   uint size;            // Size of file (bytes)
3932   uint addrs[NDIRECT+1];   // Data block addresses
3933 };
3934 
3935 // Inodes per block.
3936 #define IPB           (BSIZE / sizeof(struct dinode))
3937 
3938 // Block containing inode i
3939 #define IBLOCK(i)     ((i) / IPB + 2)
3940 
3941 // Bitmap bits per block
3942 #define BPB           (BSIZE*8)
3943 
3944 // Block containing bit for block b
3945 #define BBLOCK(b, ninodes) (b/BPB + (ninodes)/IPB + 3)
3946 
3947 // Directory is a file containing a sequence of dirent structures.
3948 #define DIRSIZ 14
3949 
3950 struct dirent {
3951   ushort inum;
3952   char name[DIRSIZ];
3953 };
3954 
3955 
3956 
3957 
3958 
3959 
3960 
3961 
3962 
3963 
3964 
3965 
3966 
3967 
3968 
3969 
3970 
3971 
3972 
3973 
3974 
3975 
3976 
3977 
3978 
3979 
3980 
3981 
3982 
3983 
3984 
3985 
3986 
3987 
3988 
3989 
3990 
3991 
3992 
3993 
3994 
3995 
3996 
3997 
3998 
3999 
