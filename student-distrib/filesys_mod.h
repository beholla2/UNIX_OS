/*	*********************************************************
	# FILE NAME: filesys_mod.h
	# PURPOSE: header for filesys_mod.c
	# AUTHOR: Queeblo OS
	# CREATED: 10/29/2014
	# MODIFIED: 11/03/2014
	********************************************************* */
#ifndef _FILESYS_MOD_H
#define _FILESYS_MOD_H

#include "types.h"
#include "multiboot.h"
#include "lib.h"



#define FNAME_LENGTH 32
#define NUM_RESERVED_BB 52
#define NUM_RESERVED_DE 24
#define DENTRY_SIZE 64
#define FS_FIELD_SIZE 4
#define KB4 4096
#define MAX_DBLOCKS_PER_FILE 1023
#define MAX_DENTRIES 63

/* AW directory entry struct */
typedef struct dentry {
	char name[FNAME_LENGTH + 1];		/* up to 32 characters plus room for null-termination */
	uint8_t type;						/* file type; 0=user-level access to RTC, "1 for the directory", "2 for a regular file" */
	uint32_t inode;						/* index node number; "only meaningful for regular files" */
	uint8_t reserved[NUM_RESERVED_DE];		/* 24 bytes reserved */	
} dentry_t;

/* AW boot block struct */
typedef struct boot_block {
	uint32_t* filesys_ptr;				/* pointer to the start of the file system (itself!) */
	uint32_t num_dir_entries;			/* number of directory entries */
	uint32_t num_inodes;				/* number of inodes N */
	uint32_t num_data_blocks;			/* number of data blocks D */
	uint8_t reserved[52];				/* 52 bytes reserved */
	uint8_t* dir_entries[MAX_DENTRIES];	/* static array of pointers to directory entries in the file system */
	uint32_t* inode_blocks;				/* pointer to first 4-byte long in first inode block */
	uint8_t* data_blocks;				/* pointer to first byte in first data block */
} boot_block_t;

extern boot_block_t fs_info;			/* global struct to collect data from boot block */

/* AW inode struct */
typedef struct index_node {
	uint32_t file_length;			/* length of file in bytes */
	uint32_t dblock_numbers[MAX_DBLOCKS_PER_FILE];	/* array of data block numbers that correspond to this file */
} inode_t;

/************ function declarations *************************/
int32_t open_file();

int32_t read_file(int32_t fd, void* buf, int32_t nbytes);

int32_t write_file(uint8_t* fname, void* buf, int32_t nbytes);

int32_t close_file();

int32_t open_directory();

int32_t read_directory(uint8_t* fname, void* buf, int32_t nbytes);

int32_t write_directory(uint8_t* fname, void* buf, int32_t nbytes);

int32_t close_directory();

void filesys_init(uint32_t* file_sys_start);	/* function to gather info from boot block into global struct */

/* AW helper functions from spec */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

inode_t * inode_address (uint32_t inode);



#endif /* _FILESYS_MOD_H */
