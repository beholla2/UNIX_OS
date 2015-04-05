/*	*********************************************************
	# FILE NAME: testcode.c
	# PURPOSE: Test functionality prior to hand-in
	# AUTHOR: Queeblo OS
	# CREATED: 11/14/2014
	# MODIFIED: 11/14/2014
	********************************************************* */
#include "testcode.h"



int testCP1_and_CP2()
{
	uint8_t* dummy;
	int ret_val = 0;

	/* print file system meta information */
	clear();
	printf("    starting address of file system: 0x%#x\n", fs_info.filesys_ptr);
	printf("    number of directory entries: %d\n", fs_info.num_dir_entries);
	printf("    number of inodes: %d\n", fs_info.num_inodes);
	printf("    number of data blocks: %d\n", fs_info.num_data_blocks);

	int i = 1;
	int k;

	/* print file names */
	dentry_t test_dentry;
	for(i = 0; i < fs_info.num_dir_entries; i++){
		k = read_dentry_by_index(i, &test_dentry);
		printf("    file %d: %s inode#: %d\n", i, test_dentry.name, test_dentry.inode);
	}

	/************ test read_dentry_by_name ************************/
	clear();
	dentry_t test_dentry2;
	printf("    Testing read_dentry_by_name...\n");
	char filename[] = "verylargetxtwithverylongname.tx";
	i = read_dentry_by_name((uint8_t*)filename, &test_dentry2);
	if(i == 0){
		printf("    File Name: %s\n", test_dentry2.name);
		printf("    File Type: %d\n", test_dentry2.type);
		printf("    Inode Number: %d\n", test_dentry2.inode);
	} else printf("    Error reading file: '%s'\n", filename);

	/************** test read_dentry_by_index **********************/
	printf("    Testing read_dentry_by_index...\n");
	uint32_t index = 7;
	i = read_dentry_by_index(index, &test_dentry2);
	if(i == 0){
		printf("    File Name: %s\n", test_dentry2.name);
		printf("    File Type: %d\n", test_dentry2.type);
		printf("    Inode Number: %d\n", test_dentry2.inode);
	} else printf("    Error reading file at index: %d\n", index);

	/******************** test read_data ********************************/
	printf("    Testing read_data....\n");
	char filename2[] = "verylargetxtwithverylongname.tx";
	i = read_dentry_by_name((uint8_t*)filename2, &test_dentry2);
	uint32_t inode = test_dentry2.inode;
	uint32_t offset = 0;
	uint8_t buf[2];
	uint32_t length = 2;
	printf("    reading data from file '%s' (inode # %d)\n", test_dentry2.name, test_dentry2.inode);
	clear();
	i = read_data(inode, offset, buf, length);
	if(i != -1){
		printf("    num bytes read: %d\n", i);
		
		printf("%s\n", buf);

		/* 
		int j;
		for(j = 0; j < i; j++){
			printf("    byte %d: %c\n", j, buf[j]);
		} */
	} else printf("    Error reading data from file\n");


	/************ Test read_directory ***************************/
	clear();
	printf("    Testing read_directory...\n");
	uint8_t dir_buf[33];
	uint32_t dir_nbytes;
	int a;
	for(a = 0; a < 20; a++)
	{
		int32_t dir_ret = read_directory(dummy, dir_buf, dir_nbytes);

		printf("    Number of bytes read: %d  ", dir_ret);
		if(dir_ret > 0)
			printf("File name: '%s' ", dir_buf);
		printf("\n");
	}

	/************ Test read_file ***************************/
	clear();
	printf("    Testing read_file...\n");
	uint8_t file_buf[100];
	uint32_t file_nbytes;
	file_nbytes = 100;
	uint32_t file_ret;
	

	/* AW we changed the interface for this function so I'm commenting it out */
	//uint8_t file_name[] = "verylargetxtwithverylongname.tx";
	//file_ret = read_file(file_name, file_buf, file_nbytes);
	
	printf("    Number of bytes read: %d\n", file_ret);
	if(file_ret > 0){
		printf(" data from buffer: ");
		for(a = 0; a < file_nbytes; a++){
			printf("%c", file_buf[a]);
		}
	}
	printf("\n");

	/*********** Test RTC by changing frequencies ************/
	/* testing basic rtc functions */
	i = 4;
	rtc_open();
	rtc_write(dummy, &i,4);
	i = 1025;
	if(rtc_write(dummy, &i,4) == -1)
		printf("rtc_write(1025,4) returns -1\n");
	rtc_read(dummy, 0,0);
	if(rtc_close() == 0)
		printf("rtc_close returns 0\n");

	/* setting flag to change freq */
	rtc_set_flag();
	



	/* explicitly dereference null ptr */
/*	int* null_ptr = 0x0;
	*null_ptr = 314;
*/
	return ret_val;

}


int testCP1()
{
	printf("Testing Checkpoint 1............\n");
	int ret_val = testCP1_and_CP2();

	return ret_val;
}


int testCP2()
{
	printf("Testing Checkpoint 2............\n");
	int ret_val = testCP1_and_CP2();

	return ret_val;
}

int testCP3_execute()
{
int is_passing = 1;
	int ret_val;
	printf("CP3: Testing execute()...\n");
	
	ret_val = execute((uint8_t*)"");
	if(ret_val != -1){
		is_passing = 0;
		printf("    execute: FAILED empty string command\n");
	} else
		printf("    execute: passed empty string command\n");

	ret_val = execute((uint8_t*)"    ");
	if(ret_val != -1){
		is_passing = 0;
		printf("    execute: FAILED all spaces command\n");
	} else
		printf("    execute: passed all spaces command\n");

	ret_val = execute((uint8_t*)"catb");
	if(ret_val != -1){
		is_passing = 0;
		printf("    execute: FAILED bad filename command\n");
	} else
		printf("    execute: passed bad filename command\n");

	ret_val = execute((uint8_t*)"garbage");
	if(ret_val != -1){
		is_passing = 0;
		printf("    execute: FAILED bad filename command 2\n");
	} else
		printf("    execute: passed bad filename command 2\n");

	ret_val = execute((uint8_t*)"lots of words and spaces");
	if(ret_val != -1){
		is_passing = 0;
		printf("    execute: FAILED mult words bad command\n");
	} else
		printf("    execute: passed mult words bad command\n");

	ret_val = execute((uint8_t*)"frame0.txt");
	if(ret_val != -1){
		is_passing = 0;
		printf("    execute: test non-executable- FAILED\n");
	} else
		printf("    execute: test non-executable- passed\n");

	ret_val = execute((uint8_t*)"ls");
	if(ret_val != -1){
		is_passing = 1;
		printf("    execute: test 'ls'- ret_val = %d\n", ret_val);
	} else
		printf("    execute: test 'ls'- ret_val = %d\n", ret_val);

	return is_passing;

}

int testCP3()
{
	printf("Testing Checkpoint 3............\n");
	int ret_val = 0;

	ret_val = testCP3_execute();

	return ret_val;
}

int testCP4()
{
	printf("Testing Checkpoint 4............\n");
	int ret_val = 0;

	return ret_val;
}

int testCP5()
{
	printf("Testing Checkpoint 5............\n");
	int ret_val = 0;

	return ret_val;
}


/* run_tests(uint8_t* test_name)
 * INPUTS:			test_name - string indicating which test to run
 * RETURN VALUE:	0 on success
 * PURPOSE: 		Main function to call specific tests
 */
int run_tests(int8_t* test_name)
{
	uint32_t n = strlen((int8_t*)test_name);
	int ret_val;
	
	if ( (strncmp((int8_t*)test_name, "cp1", n) == 0) ||
		 (strncmp((int8_t*)test_name, "checkpoint1", n) == 0) )
			ret_val = testCP1();
	else if ( (strncmp((int8_t*)test_name, "cp2", n) == 0) ||
			  (strncmp((int8_t*)test_name, "checkpoint2", n) == 0) )
			ret_val = testCP2();
	else if ( (strncmp((int8_t*)test_name, "cp3", n) == 0) ||
			  (strncmp((int8_t*)test_name, "checkpoint3", n) == 0) )
			ret_val = testCP3();
	else if ( (strncmp((int8_t*)test_name, "cp4", n) == 0) ||
			  (strncmp((int8_t*)test_name, "checkpoint4", n) == 0) )
			ret_val = testCP4();
	else if ( (strncmp((int8_t*)test_name, "cp5", n) == 0) ||
			  (strncmp((int8_t*)test_name, "final", n) == 0) ||
			  (strncmp((int8_t*)test_name, "checkpoint5", n) == 0) )
			ret_val = testCP5();
	
	return ret_val;
}
