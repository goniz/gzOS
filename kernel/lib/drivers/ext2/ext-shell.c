/* =============
 * ext shell
 * AUTHOR : CVS
 * =============
 */

#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <platform/cpu.h>

#include "inc/types.h"
#include "inc/superblock.h"
#include "inc/blockgroup_descriptor.h"
#include "inc/inode.h"
#include "inc/directoryentry.h"

#define DEBUG 1

#define debug(...) \
            do { if (DEBUG) printf("<debug> " __VA_ARGS__); fflush(stdout); } while (0)


struct os_superblock_t *superblock;
struct os_blockgroup_descriptor_t *blockgroup;
struct os_inode_t *inodes;

void read_superblock(int fd)
{
	superblock = malloc(sizeof(struct os_superblock_t));
	assert(superblock != NULL);
       
	assert(lseek(fd, (off_t)1024, SEEK_SET) == (off_t)1024);
	assert(read(fd, (void *)superblock, sizeof(struct os_superblock_t)) == sizeof(struct os_superblock_t));

	endian_set(le32_to_cpu, superblock->s_inodes_count);
	endian_set(le32_to_cpu, superblock->s_blocks_count);
	endian_set(le32_to_cpu, superblock->s_r_blocks_count);
	endian_set(le32_to_cpu, superblock->s_free_blocks_count);
	endian_set(le32_to_cpu, superblock->s_free_inodes_count);
	endian_set(le32_to_cpu, superblock->s_first_data_block);
	endian_set(le32_to_cpu, superblock->s_log_block_size);
	endian_set(le32_to_cpu, superblock->s_log_frag_size);
	endian_set(le32_to_cpu, superblock->s_blocks_per_group);
	endian_set(le32_to_cpu, superblock->s_frags_per_group);
	endian_set(le32_to_cpu, superblock->s_inodes_per_group);
	endian_set(le32_to_cpu, superblock->s_mtime);
	endian_set(le32_to_cpu, superblock->s_wtime);
	endian_set(le16_to_cpu, superblock->s_mnt_count);
	endian_set(le16_to_cpu, superblock->s_max_mnt_count);
	endian_set(le16_to_cpu, superblock->s_magic);
	endian_set(le16_to_cpu, superblock->s_state);
	endian_set(le16_to_cpu, superblock->s_errors);
	endian_set(le16_to_cpu, superblock->s_minor_rev_level);
	endian_set(le32_to_cpu, superblock->s_lastcheck);
	endian_set(le32_to_cpu, superblock->s_checkinterval);
	endian_set(le32_to_cpu, superblock->s_creator_os);
	endian_set(le32_to_cpu, superblock->s_rev_level);
	endian_set(le16_to_cpu, superblock->s_def_resuid);
	endian_set(le16_to_cpu, superblock->s_def_resgid);
	endian_set(le32_to_cpu, superblock->s_first_ino);
	endian_set(le16_to_cpu, superblock->s_inode_size);
	endian_set(le16_to_cpu, superblock->s_block_group_nr);
	endian_set(le32_to_cpu, superblock->s_feature_compat);
	endian_set(le32_to_cpu, superblock->s_feature_incompat);
	endian_set(le32_to_cpu, superblock->s_feature_ro_compat);
	endian_set(le32_to_cpu, superblock->s_algo_bitmap);
	endian_set(le32_to_cpu, superblock->s_journal_inum);
	endian_set(le32_to_cpu, superblock->s_journal_dev);
	endian_set(le32_to_cpu, superblock->s_last_orphan);
	endian_set(le32_to_cpu, superblock->s_default_mount_options);
	endian_set(le32_to_cpu, superblock->s_first_meta_bg);
}

void read_blockgroup(int fd)
{
	blockgroup = malloc(sizeof(struct os_blockgroup_descriptor_t));
	assert(blockgroup != NULL);
       
	assert(lseek(fd, (off_t)2048, SEEK_SET) == (off_t)2048);
	assert(read(fd, (void *)blockgroup, sizeof(struct os_blockgroup_descriptor_t)) == sizeof(struct os_blockgroup_descriptor_t));

	endian_set(le32_to_cpu, blockgroup->bg_block_bitmap);
	endian_set(le32_to_cpu, blockgroup->bg_inode_bitmap);
	endian_set(le32_to_cpu, blockgroup->bg_inode_table);
	endian_set(le16_to_cpu, blockgroup->bg_free_blocks_count);
	endian_set(le16_to_cpu, blockgroup->bg_free_inodes_count);
	endian_set(le16_to_cpu, blockgroup->bg_used_dirs_count);
}

void read_inodeTable(int fd)
{
	// preparing to cache inode table in inodes
    printf("s_inodes_count: %d, s_inodes_size: %d\n", superblock->s_inodes_count, superblock->s_inode_size);
	inodes = (struct os_inode_t*)malloc(superblock->s_inodes_count*superblock->s_inode_size);
	assert(inodes != NULL);

	// seek to start of inode_table
	assert(lseek(fd, (off_t)(blockgroup->bg_inode_table*1024), SEEK_SET) == (off_t)(blockgroup->bg_inode_table*1024));

#if 1
	// read-in every inode into cache
	for(int i=0; i<superblock->s_inodes_count;i++) {
		lseek(fd, (off_t)superblock->s_inode_size , SEEK_CUR);

		struct os_inode_t* inode = &inodes[i];
		assert(read(fd, inode, sizeof(struct os_inode_t)) == sizeof(struct os_inode_t));

		endian_set(le16_to_cpu, inode->i_mode);
		endian_set(le16_to_cpu, inode->i_uid);
		endian_set(le32_to_cpu, inode->i_size);
		endian_set(le32_to_cpu, inode->i_atime);
		endian_set(le32_to_cpu, inode->i_ctime);
		endian_set(le32_to_cpu, inode->i_mtime);
		endian_set(le32_to_cpu, inode->i_dtime);
		endian_set(le16_to_cpu, inode->i_gid);
		endian_set(le16_to_cpu, inode->i_links_count);
		endian_set(le16_to_cpu, inode->i_blocks);
		endian_set(le32_to_cpu, inode->i_flags);

		for (int j = 0; j < 15; j++) {
			endian_set(le32_to_cpu, inode->i_block[j]);
		}

		endian_set(le32_to_cpu, inode->i_generation);
		endian_set(le32_to_cpu, inode->i_file_acl);
		endian_set(le32_to_cpu, inode->i_dir_acl);
		endian_set(le32_to_cpu, inode->i_faddr);

        printf("inode: %d size %d\n", i, inode->i_size);
	}
#else

	assert(read(fd, (void *)inodes, 0x40000) == 0x40000);

#endif

}

void printInodeType(int inode_type)
{
	switch(inode_type)
	{
	case 1:
		printf("-");
		break;
	case 2:
		printf("d");
		break;
	case 3:
		printf("c");
		break;
	case 4:
		printf("b");
		break;
	case 5:
		printf("B");
		break;
	case 6:
		printf("S");
		break;
	case 7:
		printf("l");
		break;
	default:
		printf("X");
		break;
	}
}

void printInodePerm(int fd, int inode_num)
{
	//int curr_pos = lseek(fd, 0, SEEK_CUR);
	short int mode = inodes[inode_num-1].i_mode;

	mode & EXT2_S_IRUSR ? printf("r") : printf("-");
	mode & EXT2_S_IWUSR ? printf("w") : printf("-");
	mode & EXT2_S_IXUSR ? printf("x") : printf("-");
	mode & EXT2_S_IRGRP ? printf("r") : printf("-");
	mode & EXT2_S_IWGRP ? printf("w") : printf("-");
	mode & EXT2_S_IXGRP ? printf("x") : printf("-");
	mode & EXT2_S_IROTH ? printf("r") : printf("-");
	mode & EXT2_S_IWOTH ? printf("w") : printf("-");
	mode & EXT2_S_IXOTH ? printf("x") : printf("-");

	printf("\t");

	//lseek(fd, curr_pos, SEEK_SET);
}


/* findInodeByName
 * 
 * Params:
 * int fd		fd to img file
 * char* filename	name of file to find
 * int filetype		filetype os_direntry_t->file_type
 *
 * Returns:
 * int			valid inode-num if found, else -1.
 */

int findInodeByName(int fd, int base_inode_num, char* filename, int filetype)
{
	char* name;
	int curr_inode_num;
	int curr_inode_type;

	debug("data block addr\t= 0x%x\n", inodes[base_inode_num-1].i_block[0]);

	struct os_direntry_t* dirEntry = malloc(sizeof(struct os_direntry_t));
	assert (dirEntry != NULL);

	off_t offset = (off_t)(inodes[base_inode_num - 1].i_block[0] * 1024);
	assert(lseek(fd, offset, SEEK_SET) == offset);
	assert(read(fd, (void *)dirEntry, sizeof(struct os_direntry_t)) == sizeof(struct os_direntry_t));

	endian_set(le32_to_cpu, dirEntry->inode);
	endian_set(le16_to_cpu, dirEntry->rec_len);

	while (dirEntry->inode) {

		name = (char*)malloc(dirEntry->name_len+1);
		memcpy(name, dirEntry->file_name, dirEntry->name_len);
		name[dirEntry->name_len+1] = '\0';

		curr_inode_num = dirEntry->inode;
		curr_inode_type = 2;//dirEntry->file_type;

		if (filetype == curr_inode_type) {
			if(!strcmp(name, filename)) {
                free(dirEntry);
                free(name);
				return(curr_inode_num);
			}
		}

        free(name);

		lseek(fd, (dirEntry->rec_len - sizeof(struct os_direntry_t)), SEEK_CUR);
		assert(read(fd, (void *)dirEntry, sizeof(struct os_direntry_t)) == sizeof(struct os_direntry_t));

        endian_set(le32_to_cpu, dirEntry->inode);
        endian_set(le16_to_cpu, dirEntry->rec_len);
	}

    free(dirEntry);
	return(-1);	
}

void saveInode(int fd, int inode_num, char* filename)
{
	int wfd = open(filename, O_RDWR | O_CREAT);
	if (wfd == -1) {
		printf("Could NOT open file \"%s\"\n", filename);
	}

	char* buffer = malloc(1024);
	assert (buffer != NULL);

	off_t offset = (off_t)(inodes[inode_num - 1].i_block[0] * 1024);
	assert(lseek(fd, offset, SEEK_SET) == offset);
	assert(read(fd, (void *)buffer, 1024) == 1024);

	write(wfd, buffer, inodes[inode_num-1].i_size);
	close(wfd);

}

void ls(int fd, int base_inode_num)
{
	char* name;
	int curr_inode_num;
	int curr_inode_type;
    int inode = base_inode_num;
    int block = inodes[inode].i_block[0];

	debug("inode %d data block addr\t= 0x%x\n", inode, block);

	struct os_direntry_t* dirEntry = malloc(sizeof(struct os_direntry_t));
	assert (dirEntry != NULL);

	off_t offset = (off_t)(block * 1024);
	assert(lseek(fd, offset, SEEK_SET) == offset);
	assert(read(fd, (void *)dirEntry, sizeof(struct os_direntry_t)) == sizeof(struct os_direntry_t));

    endian_set(le32_to_cpu, dirEntry->inode);
    endian_set(le16_to_cpu, dirEntry->rec_len);

    printf("inode: %d\n", dirEntry->inode);
    printf("reclen: %d\n", dirEntry->rec_len);

	 while (dirEntry->inode) {

		name = (char*)malloc(dirEntry->name_len+1);
		memcpy(name, dirEntry->file_name, dirEntry->name_len);
		name[dirEntry->name_len+1] = '\0';

		curr_inode_num = dirEntry->inode;
		curr_inode_type = 2; //dirEntry->file_type;

		lseek(fd, (dirEntry->rec_len - sizeof(struct os_direntry_t)), SEEK_CUR);
		assert(read(fd, (void *)dirEntry, sizeof(struct os_direntry_t)) == sizeof(struct os_direntry_t));

        endian_set(le32_to_cpu, dirEntry->inode);
        endian_set(le16_to_cpu, dirEntry->rec_len);

		if (name[0] == '.') {
			if ( name[1]=='.' || name[1]=='\0')
				continue;
		} else {
			debug("rec_len\t\t= %d\n", dirEntry->rec_len);
			debug("dirEntry->inode\t= %d\n",dirEntry->inode);
			printInodeType(curr_inode_type);
			printInodePerm(fd, curr_inode_num);
			printf("%d\t", curr_inode_num);
			printf("%s\t", name);
			printf("\n");
		}
	} 

	return;
}

void cp(int fd, int base_inode_num)
{
	char filename[255];
	int ret;

	//printf("Enter filename:");
	scanf("%s", filename);

	ret = findInodeByName(fd, base_inode_num, filename, EXT2_FT_REG_FILE);
	debug("findInodeByName=%d\n", ret);

	if(ret==-1) {
		printf("File %s does not exist\n", filename);
	} else {
		printf("Saving file %s\n", filename);
		saveInode(fd, ret, filename);
	}

}

int cd(int fd, int base_inode_num)
{
	char dirname[255];
	int ret;

	//printf("Enter directory name:");
	scanf("%s", dirname);

	ret = findInodeByName(fd, base_inode_num, dirname, EXT2_FT_DIR);
	debug("findInodeByName=%d\n", ret);

	if(ret==-1) {
		printf("Directory %s does not exist\n", dirname);
		return(base_inode_num);
	} else {
		printf("Now in directory %s\n", dirname);
		return(ret);
	}

}

int extShell(int fd )
{
	char cmd[4];
	static int pwd_inode = 0;

	printf("ext-shell$ ");
    fflush(NULL);
	scanf("%s", cmd);

	debug("cmd=%s\n", cmd);

	if(!strcmp(cmd, "q")) {
		return(-1);

	} else if(!strcmp(cmd, "ls")) {
		ls(fd, pwd_inode);

	} else if(!strcmp(cmd, "cd")) {
		pwd_inode = cd(fd, pwd_inode);

	} else if(!strcmp(cmd, "cp")) {
		cp(fd, pwd_inode);

	} else {
		printf("Unknown command: %s\n", cmd);
		return(-EINVAL);
	}

	return(0);
}

int main(int argc, char **argv)
{
	// open up the disk file
//	if (argc !=2) {
//	printf("usage:  ext-shell <file.img>\n");
//		return -1;
//	}

	int fd = open("/pflash/userdata", O_RDONLY);
	if (fd == -1) {
		printf("Could NOT open file \"%s\"\n", argv[1]);
		return -1; 
	}

	// reading superblock
	read_superblock(fd);
	printf("block size \t\t= %d bytes\n", 1<<(10 + superblock->s_log_block_size));
	printf("inode count \t\t= 0x%x\n", superblock->s_inodes_count);
	printf("inode size \t\t= 0x%x\n", superblock->s_inode_size);

	// reading blockgroup
	read_blockgroup(fd);
	printf("inode table address \t= 0x%x\n", blockgroup->bg_inode_table);
	printf("inode table size \t= %dKB\n", (superblock->s_inodes_count*superblock->s_inode_size)>>10);

	// reading inode table
	read_inodeTable(fd);

	while(1) {
		// extShell waits for one cmd and executes it.
		// returns 0 on successfull execution of command,
		// returns -EINVAL on unknown command
		// returns -1 if cmd="q" i.e. quit.
		if ( extShell(fd)==-1 )
			break;
	}

	printf("\n\nQuitting ext-shell.\n\n");
	return(0);
}
