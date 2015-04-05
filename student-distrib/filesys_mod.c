/*	*********************************************************
	# FILE NAME: filesys_mod.c
	# PURPOSE: support operations on the provided file system image
	# AUTHOR: Queeblo OS
	# CREATED: 10/29/2014
	# MODIFIED: 11/03/2014
	********************************************************* */

#include "filesys_mod.h"
#include "syscalls.h"


/* AW declare global boot block struct */
boot_block_t fs_info;

/* filesys_init(multiboot_info_t* mbi)
 * INPUTS:			mbi - pointer to the multiboot information struct
 * RETURN VALUE:	none
 * PURPOSE: 		Initialize a global struct with meta information about the file
 *					system such as the starting address, number of directory entries,
 *					number of inodes, etc.
 */
void filesys_init(uint32_t* file_sys_start)
{
//	int i;
	uint32_t d_idx;				/* index to a directory entry in the boot block */
	uint8_t* filename_ptr;		/* ptr to file names in dir dentries */

	//module_t* mod = (module_t*)mbi->mods_addr;				/* for starters, assume 1 module only */
	fs_info.filesys_ptr =  file_sys_start;		/* according to boot screen this is 0x0040D000 */

	fs_info.num_dir_entries = *fs_info.filesys_ptr;		/* parse number of directory entries in filesys_img */
	fs_info.filesys_ptr++;
	fs_info.num_inodes = *fs_info.filesys_ptr;			/* parse number of inodes in filesys_img */
	fs_info.filesys_ptr++;
	fs_info.num_data_blocks = *fs_info.filesys_ptr;		/* parse number of data blocks in filesys_img */
	fs_info.filesys_ptr++;
	
	fs_info.filesys_ptr += 13;						/* skip over 52 reserved bytes (13*4 bytes = 52 bytes) */
	filename_ptr = (uint8_t*)fs_info.filesys_ptr;	/* now we're pointing to file names */

	/* parse pointers to directory entries */
	for(d_idx = 0; d_idx < MAX_DENTRIES; d_idx++){
		fs_info.dir_entries[d_idx] = filename_ptr;
		filename_ptr += DENTRY_SIZE;						/* move pointer to next directory entry */
	}
	
	fs_info.filesys_ptr =  file_sys_start;					/* reset file system pointer to start of file system */
	fs_info.inode_blocks = (uint32_t*)(filename_ptr);		/* ptr to first inode block */
	fs_info.data_blocks = ( (uint8_t*)(fs_info.inode_blocks) + fs_info.num_inodes*KB4 );	/* ptr to first data block */

}


/* open_file()
 * INPUTS:	none
 * RETURN VALUE: 0
 * PURPOSE: Place holder function to be integrated into system calls
 */
int32_t open_file()
{
	return 0;
}


/* open_directory()
 * INPUTS:	none
 * RETURN VALUE: 0
 * PURPOSE: Place holder function to be integrated into system calls
 */
int32_t open_directory()
{
	return 0;
}


/* read_file(uint8_t fname, void* buf, int32_t nbytes)
 * INPUTS:			fname - filename to read from
 *					buf - pointer to buffer to be filled with data from the file
 * 					nbytes - number of bytes to read
 * RETURN VALUE: 	The number of bytes read into the buffer
 * PURPOSE: 		Read consecutive bytes from the desginated file into the
 *					provided buffer and return the number of bytes read.
 */
int32_t read_file(int32_t fd, void* buf, int32_t nbytes)
{
	dentry_t dentry_one;
	int32_t ret_val;

	process_control_block_t* current_pblock = (process_control_block_t*)(ADDR_8MB - process_count*ADDR_8KB);
	uint8_t* fname = current_pblock->fde[fd].file_name;		/* AW get filename from fd struct */
	uint32_t offset = current_pblock->fde[fd].file_pos;		/* AW get file position from fd struct */

	read_dentry_by_name (fname, &dentry_one);		/* fill dentry struct */

	ret_val = read_data (dentry_one.inode, offset, (uint8_t*)buf, nbytes);	/* read data from file */
	current_pblock->fde[fd].file_pos += ret_val;

	return ret_val;
}


/* read_directory(void* buf, int32_t nbytes)
 * INPUTS:			buf - pointer to buffer to be filled with file name
 * 					nbytes - unused
 * RETURN VALUE: 	The number of bytes read into the buffer
 * PURPOSE: 		Read a filename from the directory into a buffer,
 *					with consecutive reads putting consecutive filenames into
 *					the buffer (overwriting the previous filename).
 *					After the last child is reached, further calls to the 
 *					function return 0.
 */
int32_t read_directory(uint8_t* fname, void* buf, int32_t nbytes)
{
	dentry_t dentry_one;

	static uint32_t dir_read_count = 0;

	int32_t bytes_read;

	int ret_value = read_dentry_by_index (dir_read_count, &dentry_one);	/* fill dentry struct */

	if(ret_value == -1)
	{
		dir_read_count = 0;
		return 0;
	}
	else
	{

		dir_read_count++;

		strcpy(((int8_t*)buf), ((int8_t*)&dentry_one.name));	/* put file name into buffer */

		bytes_read = strlen(((int8_t*)buf));
	}

	return bytes_read;
}


/* write_file(void* buf, int32_t nbytes)
 * INPUTS:			buf - unused
 * 					nbytes - unused
 * RETURN VALUE: -1 because the file system is read-only
 * PURPOSE: Place holder function to be integrated into system calls
 */
int32_t write_file(uint8_t* fname, void* buf, int32_t nbytes)
{
	return -1;
}


/* write_directory(void* buf, int32_t nbytes)
 * INPUTS:			buf - unused
 * 					nbytes - unused
 * RETURN VALUE: -1 because the file system is read-only
 * PURPOSE: Place holder function to be integrated into system calls
 */
int32_t write_directory(uint8_t* fname, void* buf, int32_t nbytes)
{
	return -1;
}

/* close_file()
 * INPUTS:	none
 * RETURN VALUE: 0
 * PURPOSE: Place holder function to be integrated into system calls
 */
int32_t close_file()
{
	return 0;
}

/* close_directory()
 * INPUTS:	none
 * RETURN VALUE: 0
 * PURPOSE: Place holder function to be integrated into system calls
 */
int32_t close_directory()
{
	return 0;
}

/************** AW helper functions from spec ******************/

/* Read Directory Entry by Name
 * read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
 * INPUTS:	fname - name of the file to read
 *			dentry - pointer to struct to fill with directory entry information
 * RETURN VALUE: 0 means success; -1 means failure
 * PURPOSE: Given a file name and a pointer to a (empty) dentry struct in memory,
 *			this function searches the through the directory entries in the file system
 *			for the named file and populates the dentry struct with info from the
 *			directory entry such as filename, inode #, and file type
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
{
	if( fname == NULL || dentry == NULL)
		return -1;

	int check, compare;
	uint8_t* fs_dentry_ptr;
	uint8_t fname_len = strlen((int8_t*)fname);
	uint8_t dentry_filename_len;
	uint8_t* dentry_filename;
	
	//Check every entry to check name
		/* Note that we are assuming (here or somewhere) that all dir entries are packed
	 	 * consecutively at the beginning of the directory entry array in the boot block */
	for(check = 0; check < fs_info.num_dir_entries; check++)
	{
		//Check if name is the same

		dentry_filename = fs_info.dir_entries[check];
		dentry_filename_len = strlen((int8_t*)dentry_filename);
		
		compare = 1;

		if(dentry_filename_len == fname_len)
			compare = strncmp( (int8_t*)(fname), (int8_t*)(fs_info.dir_entries[check]), fname_len );
		
		if(compare == 0)
		{
			//Create a copy of dentry at the given check into the pointer
			fs_dentry_ptr = fs_info.dir_entries[check];

			//Copy every element in the struct
			strcpy((int8_t*)(dentry->name), (int8_t*)fname );	/* file name */
			fs_dentry_ptr += FNAME_LENGTH;
			dentry->type = *fs_dentry_ptr;					/* file type */
			fs_dentry_ptr += FS_FIELD_SIZE;
			dentry->inode = *((uint32_t*)(fs_dentry_ptr));	/* index node number */
		
			//Success
			return 0;
		}
	}
	
	//In the case of error
	return -1;
}


/* Read Directory Entry by Index
 * read_dentry_by_index(uint32_t index, dentry_t* dentry)
 * INPUTS:	index - the index of the directory entry to read
 *			dentry - pointer to struct to fill with directory entry information
 * RETURN VALUE: 0 means success; -1 means failure
 * PURPOSE: Given an index and a pointer to a (empty) dentry struct in memory,
 *			this function gets the name of the file specified by the index and populates
 * 			the dentry struct with info from the directory entry such as filename, inode #, and file type
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
{
	if(index < 0 || index >= fs_info.num_dir_entries || dentry == NULL)
		return -1;

	//Create a copy of dentry at the given check into the pointer
	uint8_t* fs_dentry_ptr;
	fs_dentry_ptr = fs_info.dir_entries[index];

	//Copy every element in the struct
	strcpy((int8_t*)(dentry->name), (int8_t*)fs_dentry_ptr );	/* file name */
	fs_dentry_ptr += FNAME_LENGTH;
	dentry->type = *fs_dentry_ptr;					/* file type */
	fs_dentry_ptr += FS_FIELD_SIZE;
	dentry->inode = *((uint32_t*)(fs_dentry_ptr));	/* index node number */

	//Success
	return 0;
	
}


/* Read Data
 * read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 * INPUTS:	inode - inode number specifying the file to read data from
 *			offset - where to start reading from (number of bytes from start of file)
 *			buf - pointer to buffer to be filled with data
 *			length - number of bytes to read from file
 * RETURN VALUE: number of bytes of data that was read; -1 means failure
 * PURPOSE: Reads [length] number of bytes from the file (specified by [inode]) into
 *			the provided buffer.  Starting point for read is at offset.
 *			This function reads data which could be stored in randomly ordered data blocks
 *			and stores it contiguously in a buffer.
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	inode_t* inode_ptr_two;
	uint32_t length_check, file_check;
	uint32_t adjusted_length;
	int check = 0;
	int block_ct = 0;
	int byte_ct = 0;
	uint32_t read_bytes = length;
	uint32_t size_total = offset + length;
	uint32_t return_bytes;
	uint8_t* offset_pointer;
	uint8_t* buffer;
	
	adjusted_length = length;
	
	//Set inode_ptr_two
	inode_ptr_two = (inode_t*)((uint8_t*)(fs_info.filesys_ptr) + KB4*(inode + 1));
	
	//Set length
	length_check = inode_ptr_two->file_length - offset;
	
	//Check the size of the file
	int size_check = KB4 * MAX_DBLOCKS_PER_FILE; // Declared in filesys_mod.h
	
	if(inode_ptr_two->file_length > size_check)
	{
		//Invalid
		return -1;
	}
	
	//Ensure file fits or trim
	file_check = inode_ptr_two->file_length;
	
	if(offset + length > inode_ptr_two->file_length)
	{
		//Trim
		adjusted_length = inode_ptr_two->file_length - offset;
		size_total = offset + adjusted_length;
	}
	
	//Check if read past file
	if(length_check <= 0)
	{
		//Simply return
		return 0;
	}
	
	//Calculate the blocks
	read_bytes = adjusted_length;
	block_ct = size_total / KB4;
	byte_ct = size_total % KB4;
	
	
	//Check the offset to see the amount of skipped blocks
	while(offset > KB4)
	{
		offset = offset - KB4;
		check++;
		block_ct--;
	}
	
	return_bytes = read_bytes;
	
	buffer = buf;
	
	while(read_bytes > 0)
	{
		if(block_ct <= 0)
		{
			//Copy the end
			offset_pointer = (uint8_t*) &fs_info.data_blocks[KB4 * inode_ptr_two->dblock_numbers[check]] + offset;
			memcpy(buffer, offset_pointer, read_bytes);
			read_bytes = 0;
						
		}
		else
		{
			//Decrement the amount of blocks
			block_ct--;
			
			//Set offset pointer
			offset_pointer = (uint8_t*) &fs_info.data_blocks[KB4 * inode_ptr_two->dblock_numbers[check]] + offset;
			memcpy(buffer, offset_pointer, (KB4 - offset));
			
			//Reset buffer and read_bytes
			buffer = buffer + KB4 - offset;
			read_bytes = read_bytes - (KB4 - offset);
			offset = 0;
			
			//Increment
			check++;
			
		}
	}
	
	//Return amount of bytes read
	return return_bytes;
}

/*
* inode_address
*
* DESCRIPTION: gives the address to a particular inode
* INPUTS: (inode) the given inode that we want the address for
* OUTPUS: the address of the inode, -1 on failure
*/
inode_t * inode_address (uint32_t inode)
{
	uint32_t start_address = (uint32_t)fs_info.filesys_ptr;
	
	inode_t * temporary;
	
	boot_block_t * get_block = (boot_block_t *) start_address;
	
	//Compute the address
	temporary = (inode_t*) (start_address + KB4 + inode * KB4);
	
	if(inode >= get_block->num_inodes)
	{
		return NULL;
	}
	
	else
	{
		return temporary;
	}

}


