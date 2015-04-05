/*	*********************************************************
	# FILE NAME: syscalls.c
	# PURPOSE: implement 10 system call functions
	# AUTHOR: Queeblo OS
	# CREATED: 11/12/2014
	# MODIFIED: 11/12/2014
	********************************************************* */

#include "syscalls.h"
#include "sched.h"

fops_functions_t fops_directory_functions;
fops_functions_t fops_file_functions;
fops_functions_t fops_rtc_functions;
fops_functions_t fops_terminal_functions;

process_queue_t process_q;
int primary_shell_count;


/*  halt(uint8_t)
 * 	INPUTS: 		status - not used
 *	OUTPUTS: 		None - although we have a return 0, we should never reach that point
 *	DESCRIPTION: 	Halt checks if there is at least one process running, if there is
 * 					then we can continue on halting, if not then we execute another shell.
 *  				If we are continuing with halt we dequeue our process queue, restore the parent process's
 *					esp and ebp, restore the parent's page directory entry, set the cr3 (flush tlb),
 *					decrement processes count, set tss's stack pointer to parent process, update ebp and esp
 * 					to prepare for leave, then leave. 
 */
int32_t halt(uint8_t status)
{
	if(process_count <= 1){
		printf("Command refused.  Cannot exit last remaining process.\n");
		process_count = 0;

		/* reinitialize process queue */
		pq_init();
		
		execute((uint8_t*)"shell");
	}



		/* TESTING DEQUEUE FUNCTION
		 *	- after pcb for one process has been filled, lets fill the process queue.
		 */
		process_control_block_t * old_pcb;
		if(process_count > 1 )
		{
			old_pcb = pq_dequeue(&process_q);
		}


	/* restore the parent's esp and ebp */
		process_control_block_t* curr_pcb = (process_control_block_t*)(ADDR_8MB - (process_count)*ADDR_8KB);
		process_control_block_t* parent_pcb = (process_control_block_t*)(curr_pcb->parent_ptr);
		uint32_t esp_parent = parent_pcb->esp;
		uint32_t ebp_parent = parent_pcb->ebp;


	/* restore parent's page directory entry */
		pd_entry_t pde;

		init_4mb_user_pde(&pde, ADDR_4MB + (process_count - 1)*ADDR_4MB);		/* populate pd entry; first user prog is at 8MB, next at 12MB, etc */
		pd[PD_IDX_USER] = pde.val;	/* enter page directory entry into page directory */

		/* set cr3 */
		set_cr3(pd);	/* (flushes TLB) */

	/* maybe zero out PCB? and kernel stack? */
   
	process_count--;

	tss.esp0 = ADDR_8MB - (process_count)*ADDR_8KB - 4;	/* set address of kernel stack pointer */

	/* update esp and ebp registers with parent's esp and ebp */
	asm volatile ("pushl %0"
				:
				: "c"(esp_parent)); /* puts parent's esp into %esp */
	asm volatile ("pushl %0"
				:
				: "c"(ebp_parent)); /* puts parent's ebp into %ebp */

	asm volatile ("leave");
	return 0;	/* we should never reach this line -- now we do b/c of leave instruction?*/
}



/*  execute(const uint8_t* command_param)
 * 	INPUTS: 		command_param - string that contains the command to execute
 *	OUTPUTS: 		returns FAIL if failed (should never happen), otherwise, return into user program.
 *	DESCRIPTION: 	Parse the command_param for arguments and program name, check if file is executable
 *					set up a page for the program, read the file, initialize the pcb, perform context switch
 * 					and return into user program.
 */
int32_t execute(const uint8_t* command_param)
{	
	int32_t ret_val, i, num_spaces;		/* ### AW */
	//HOLLAND: This can be extended up to 128 I believe??
		/* AW: Actually the array of char_space_indices was made to handle the case in which we have multiple argumets separated by spaces.
		 *		The idea was the find the location of each space and store it in array.
		 *		But according to Piazza we don't have to handle more than 1 argument.  Oh well.
		 */
	uint8_t char_space_indices[100];	/* AW assuming we have no more than 100 arguments (separated by a space) */
	uint8_t exe_buff[4];
	uint8_t command[MAX_KB_BUF];		/* AW create local array in kernel */
	process_control_block_t pcb;		/* AW moved this here */

/*	static uint32_t process_count = 0; */	/* static means this should only be zero the first time;
										 * subsequent function calls will not reset this value.
										 * Are we correct to use static here? 
										 */

   	/* check if executing a new command will exceed the max process limit */
	if(process_count >= MAX_PROCESSES){
		printf("Command refused.  Attempting to exceed max processes.\n");
		return -1;
	}


	/******* step 1 - parse the command **********************/
		if(strncmp((int8_t*)command_param, "", 2) == 0)
			return FAIL;

		/* find spaces in command */
		uint32_t c_len = strlen((int8_t*)command_param);		/* AW find c_len (length of command string) */
		memcpy((uint8_t*)command, command_param, c_len + 1);	/* AW copy command from user space to kernel space */
		num_spaces = 0;					
		for(i = 0; i < c_len; i++)
		{
			if(command[i] == ASCII_SPACE)
			{
				char_space_indices[num_spaces] = i;		/* AW record location of each space in the command string */
				num_spaces++;							/* AW count number of spaces*/
			}
		}
		
		/* parse program name */
		uint8_t program_name[FNAME_LENGTH + 1];		/* AW string containing program name with room for null-termination */
		uint32_t prog_name_len;
		
		/* AW if there are no spaces, there are no arguments*/
		if(num_spaces == 0)
			prog_name_len = c_len;
		else
			prog_name_len = char_space_indices[0];

		/* AW check for 0-length program name (invalid) */
		if(prog_name_len == 0)
			return FAIL;

		/* check for too many arguments or too many spaces */
		if(num_spaces > 1){
			printf("ERROR: Command contains too many spaces and/or too many arguments.\n");
			return FAIL;
		}

		strncpy( (int8_t*)program_name, (int8_t*)command, prog_name_len);	/* AW populate program_name string */
		program_name[prog_name_len] = '\0';									/* AW set null-termination */

		/* find program in file system */
		dentry_t program_dentry;
		ret_val = read_dentry_by_name(program_name, &program_dentry);
		if( ret_val == -1)
			return FAIL;		/* if this file name cannot be found, return -1 */



	/******* step 2 - check whether file is executable *******/
		
		/* read first 4 bytes of inode to see if command is executable */
		if( read_data(program_dentry.inode, 0, exe_buff, 4) == -1 ) {
			return FAIL; /* return -1 if file cannot be read */
		}
		/* see if executable */
		if(!(exe_buff[0] == 0x7f && exe_buff[1] == 'E' && exe_buff[2] == 'L' && exe_buff[3] == 'F')){
			printf("ERROR.  %s is not an executable file.\n", program_name);
			return FAIL;
		}

 do{
		

	/******* step 3 - paging *********************************/

		/* intialize pde */
	
		/* making space and aligning our page dir */
/*		pd_t page_dir __attribute__ ((aligned (BYTES_4KB))); */

		pd_entry_t pde;
		
		/* intialize pde */
		process_count++;		/* extern variable */
		init_4mb_user_pde(&pde, ADDR_4MB + process_count*ADDR_4MB);		/* populate pd entry; first user prog is at 8MB, next at 12MB, etc */
		pd[PD_IDX_USER] = pde.val;	/* enter page directory entry into page directory */

		/* set cr3 */
		set_cr3(pd);	/* (flushes TLB) */


	/******* step 4 - file loader ****************************/
		uint8_t* prog_buf = (uint8_t*)(ADDR_128MB + PROG_IMG_OFFSET); 		/* virtual memory address of program image will be 128 MB + program image offset */

		inode_t* prog_inode = (inode_t*)((uint8_t*)(fs_info.filesys_ptr) + KB4*(program_dentry.inode + 1)); /* getting inode of the executable */

		read_data(program_dentry.inode, 0, prog_buf, prog_inode->file_length); /* filling physical mem with program */


	/******* step 5 - initialize process control block *******/

		args_initialize(command, char_space_indices, num_spaces, &pcb);

		/* setting pcb parent */
		if(primary_shell_count >= NUM_TERMINALS)
			pcb.parent_ptr = (uint32_t)(pq_peak(&process_q)); 	/* if this is not a primary shell
																 * process we set parent pcb pointers */
		else
			pcb.parent_ptr = NULL; 		/* if primary shell process, set parent to NULL */

		/* Setting pcb terminal number */
		/* If this process has a parent, then it inherits the parent's terminal number.
		 * Otherwise, it has not parent so it's terminal is the currently displayed terminal. */
		if(pcb.parent_ptr == NULL)
			pcb.terminal_num = (NUM_TERMINALS - primary_shell_count - 1);
		else
			pcb.terminal_num = ((process_control_block_t*)(pcb.parent_ptr))->terminal_num;


		/* get EIP (bytes 24-27 from executable) from prog_buf */
		pcb.eip = prog_buf[24] + (prog_buf[25] << 8) + (prog_buf[26] << 16) + (prog_buf[27] << 24);
		
		pcb.pid = process_count;			/* setting PID from process count */
		pcb.user_esp = BOTTOM_PAGE - 4;		/* setting user esp */

		/* setting esp */
		asm volatile("movl %%esp, %0"
			:"=g"(pcb.esp)
			);

		/* setting ebp */
		asm volatile("movl %%ebp, %0"
			:"=g"(pcb.ebp)
			);
	

		/* initalizing FD array set FD array */
		init_fd(pcb);

		/* initialize stdin and stdout */
		pcb.fde[0].fop_ptr = (fops_functions_t*) &fops_terminal_functions;
		pcb.fde[0].inode = NULL;
		pcb.fde[0].file_pos = 0;
		pcb.fde[0].in_use = USE;

		pcb.fde[1].fop_ptr = (fops_functions_t*) &fops_terminal_functions;
		pcb.fde[1].inode = NULL;
		pcb.fde[1].file_pos = 0;
		pcb.fde[1].in_use = USE;


		/* after pcb has been initialized, copy to correct process-kernel-stack 
		 * Formula to calculate starting address of pcb:
		   address_of_pcb = 8MB - (8kB * process_id)
		 */
		 uint32_t address_of_pcb =  ADDR_8MB - process_count*ADDR_8KB;


		 /* copy pcb to space above the kernel stack */
		 memcpy((uint32_t*)address_of_pcb, &pcb, sizeof(pcb));


		/* Put new process on the front of the queue making it the currently running process */
		pq_enqueue_front(&process_q, (process_control_block_t*)address_of_pcb);
	
		/* process_control_block_t * tmp_pcb = pq_peak(&process_q);*/	/*statement used for debugging purposes */


		 /* dereference user space */
/*		 uint8_t* test_ptr = (uint8_t*)0x080482e8;
		 *test_ptr = 10;
		 test_ptr = (uint8_t*)0x83FFFFC;
		 *test_ptr = 15;
*/
		 if(primary_shell_count < NUM_TERMINALS){
			primary_shell_count++;
		}

	} while(primary_shell_count < NUM_TERMINALS);

	/******* step 6 - context switch *************************/
		tss.esp0 = ADDR_8MB - (process_count)*ADDR_8KB - 4;	/* set address of kernel stack pointer */
			asm volatile("cli");								/* mask interrupts */
			asm volatile("movw %0, %%ax" :: "g" (USER_DS));		/* %ax <- USER_DS */
			asm volatile("movw %%ax, %%ds" :);					/* %ds <- USER_DS */
			asm volatile("pushl %0" :: "g" (USER_DS));			/* push USER_DS */
			asm volatile("pushl %0" :: "g" (pcb.user_esp));		/* push user stack pointer */
			asm volatile("pushf");								/* push flags */
			asm volatile("popl %%eax" :);
			asm volatile("orl $0x286, %%eax" :::"eax");
			asm volatile("pushl %%eax" :);
			asm volatile("pushl %0" :: "g" (USER_CS));
			asm volatile("pushl %0" :: "g" (pcb.eip));			/* push destination eip */
			asm volatile("iret");
			asm volatile("halt_ret_label:");					/* I don't think we're using this anymore... */
			asm volatile("ret");								/* since we call return in halt */

	return FAIL;	/* we should never reach this */

}


/*
 * systemcalls_initialize
 *
 * DESCRIPTION: initializes all of the system calls for the file, directory, RTC, and terminal functions. 
 * INPUTS: None
 * OUTPUTS: None
 */

void systemcalls_initialize (void)
{		
	// File functions
	fops_file_functions.function_read = (fops_read_t)read_file;
	fops_file_functions.function_write = (fops_write_t)write_file;
	fops_file_functions.function_open = (fops_open_t)open_file;
	fops_file_functions.function_close = (fops_close_t)close_file;
	
	//Directory functions
	fops_directory_functions.function_read = (fops_read_t)read_directory;
	fops_directory_functions.function_write = (fops_write_t)write_directory;
	fops_directory_functions.function_open = (fops_open_t)open_directory;
	fops_directory_functions.function_close = (fops_close_t)close_directory;
	
	//RTC functions
	fops_rtc_functions.function_read = (fops_read_t)rtc_read;
	fops_rtc_functions.function_write = (fops_write_t)rtc_write;
	fops_rtc_functions.function_open = (fops_open_t)rtc_open;
	fops_rtc_functions.function_close = (fops_close_t)rtc_close;

	//Terminal fuctions
	fops_terminal_functions.function_read = (fops_read_t)term_read;
	fops_terminal_functions.function_write = (fops_write_t)term_write;
	fops_terminal_functions.function_open = (fops_open_t)term_open;
	fops_terminal_functions.function_close = (fops_close_t)term_close;

}

/* args_initialize(const uint8_t * command, uint8_t * char_space_indices, process_control_block_t * current_pblock)
 * INPUTS:			command
 *					char_space_indices
 *					num_spaces
 *					current_pblock
 * RETURN VALUE:	none
 * PURPOSE: 		Store the argument of a process in that process's process control block, to be accesssed later
 *					by the getargs system call.
 */ 
void args_initialize(const uint8_t * command, uint8_t * char_space_indices, int32_t num_spaces, process_control_block_t * current_pblock)
{
	int count = 0;
	int start = 0;
	
	// Test to check if there are arguments
	if(num_spaces == 0)
	{
		//No arguments
		current_pblock->argument_length = 1;
		current_pblock->argument_buffer[count] = '\0';
	}
	else	// there are arguments
	{
		// Move to the end of the executable name
		count = char_space_indices[0]; 	/* AW find first space */
		
		start = ++count;
		// Move to the end of the arguments
		while(command[count] != '\0')
		{
			count++;
		}
	
		char * pcb_args_buffer = (char *)(current_pblock->argument_buffer);
		char * command_args = (char *)(command + start);
		int mem_length = (count - start);
	
		// Extract args
		memcpy(pcb_args_buffer, command_args, mem_length);
		
		current_pblock->argument_length = mem_length + 1;
		current_pblock->argument_buffer[mem_length] = '\0';
			
		
	}
}


/*
 * read(int32_t fd, void* buf, int32_t nbytes)
 * INPUTS: (fd)file descriptor, (buf)buffer, (nbytes)bytes we want to read
 * OUTPUTS: returns the given read function, -1 on FAILURE
 * DESCRIPTION: reads the amount of bytes from the specified folder into the given buffer.
 */ 
int32_t read(int32_t fd, void* buf, int32_t nbytes)
{
	//Set the correct process control block
	process_control_block_t* current_pblock = (process_control_block_t*)(ADDR_8MB - process_count*ADDR_8KB);
	
	//Test the validity of fd entry
	if(fd >= 0 && fd < OPS_SIZE)
	{
		if(current_pblock != NULL && buf != NULL)
		{
			if((check_use(fd) & USE) != 0)
			{
				if(fd != 1)	/* AW stdout is monitor; read from monitor should fail */
				{
					//Read function
					return current_pblock->fde[fd].fop_ptr->function_read(fd, buf, nbytes);	
				}
			}
		}
	}
	
	//Should never return FAIL
	return FAIL;
}

/*
* write(int32_t fd, const void* buf, int32_t nbytes)
* INPUTS: (fd)file descriptor, (buf)buffer, (nbytes)bytes we want to read
* OUTPUTS: returns the given write function, -1 on failure
* DESCRIPTION: writes the given amount of bytes into the given buffer
*/
int32_t write(int32_t fd, const void* buf, int32_t nbytes)
{
	//Set the correct process control block
	process_control_block_t* current_pblock = (process_control_block_t*)(ADDR_8MB - process_count*ADDR_8KB);
	
	//Test the validity of fd entry
	if(fd >= 0 && fd < OPS_SIZE)
	{
		if(current_pblock != NULL && buf != NULL)
		{
			if((check_use(fd) & USE) != 0)
			{
				if(fd !=0)	/* AW stdin is keyboard; write to keyboard should fail */
				{
					//Write function
					return current_pblock->fde[fd].fop_ptr->function_write(fd, buf, nbytes);
				}
				
			}
		}
	}
	
	//Should never return FAIL
	return FAIL;
}

/*
* open(const uint8_t* filename)
* INPUTS: (filename) the file that is to be opened
* OUTPUTS: returns the file descriptor number, -1 on failure
* DESCRIPTION: calls the system call on the first free file descriptor after it is initialized
*/
int32_t open(const uint8_t* filename)
{
	//Set the correct process control block
	process_control_block_t* current_pblock = (process_control_block_t*)(ADDR_8MB - process_count*ADDR_8KB);
	
	int location;
	dentry_t file_dentry;
	
	//Check validity
	if(filename == NULL || current_pblock == NULL)
	{
		return FAIL;
	}
	
	//Find the location
	for(location = 0; location <= OPS_SIZE; location++)
	{
		//Test for amount of space
		if(location == OPS_SIZE)
		{
			return FAIL;
		}
		
		if((check_use(location) & USE) == 0)
		{
			break;
		}
	}

	//Ensure the file validity
	if(read_dentry_by_name (filename, &file_dentry) == FAIL)
	{
		return FAIL;
	}
	
	//Initialize the array
	current_pblock->fde[location].inode = NULL;
	uint32_t fname_len = strlen((int8_t*)filename);							/* AW find length of filename */
	memcpy(current_pblock->fde[location].file_name, filename, fname_len+1);	/* AW initialize fname */
	current_pblock->fde[location].file_pos = 0;
	current_pblock->fde[location].in_use = USE;
	
	//Select the correct read/write functions based off of file type
	switch(file_dentry.type)
	{
				
		//RTC
		case 0:
		current_pblock->fde[location].fop_ptr = (fops_functions_t*) &fops_rtc_functions;
		break;

		//Directory
		case 1:
		current_pblock->fde[location].fop_ptr = (fops_functions_t*) &fops_directory_functions;
		break;
		
		//Regular File
		case 2:
		current_pblock->fde[location].fop_ptr = (fops_functions_t*) &fops_file_functions;
		current_pblock->fde[location].inode = inode_address(file_dentry.inode);
		break;
		
		default:
		break;
	}
	
	//Execute the open function
	current_pblock->fde[location].fop_ptr->function_open();
	
	//Return the file descriptor number
	return location;
}

/*
* close(int32_t fd, const void* buf, int32_t nbytyes)
* INPUTS: (fd) file descriptor, (buf) NOT USED, (nbytes) NOT USED
* OUTPUTS: returns 0, -1 on failure
* DESCRIPTION: closes the file descriptor and calls the correct close function given the file type
*/
int32_t close(int32_t fd, const void* buf, int32_t nbytyes)
{
	//Set the correct process control block
	process_control_block_t* current_pblock = (process_control_block_t*)(ADDR_8MB - process_count*ADDR_8KB);
	
	//Ensure that it is actually being used
	if((check_use(fd) & USE) == 0)
	{
		return FAIL;
	}
	
	//Ensure that it is a valid file descriptor
	if(fd <= INDEX || fd >= OPS_SIZE || current_pblock == NULL)
	{
		return FAIL;
	}
	
	//If in use and valid, call close
	if(current_pblock->fde[fd].fop_ptr != NULL)
	{
		current_pblock->fde[fd].fop_ptr->function_close();
	}
	
	else
	{
		return FAIL;
	}
	
	//Reset the table entry
	current_pblock->fde[fd].fop_ptr = NULL;
	current_pblock->fde[fd].inode = NULL;
	current_pblock->fde[fd].file_pos = 0;
	current_pblock->fde[fd].in_use = 0;
	
	return SUCCESS;
}

/*
* check_use (int32_t fd)
* INPUTS: (fd) file descriptor number
* OUTPUTS: the in_use element of the given fd, -1 on failure
* DESCRIPTION: obtains the value for wheter or not the file is in use
*/
uint32_t check_use (int32_t fd)
{
	/* Set the correct process control block */
	process_control_block_t* current_pblock = (process_control_block_t*)(ADDR_8MB - process_count*ADDR_8KB);

	/* Ensure there is a given pcb */
	if(current_pblock != NULL)
	{
		/* Return the in_use element of the given fd */
		return current_pblock->fde[fd].in_use;
	}
	
	else
	{
		return FAIL;
	}

	/* Should not have to return FAIL */
	return FAIL;
}

/*
* int32_t getargs(uint8_t* buf, int32_t nbytes)
* INPUTS: (buf) buffer, (nbytes) bytes to be read
* OUTPUTS: 1 on success, -1 on failure
* DESCRIPTION: error checks and fills in arguments
*/
int32_t getargs(uint8_t* buf, int32_t nbytes)
{
	/* Set the correct process control block */
	process_control_block_t* current_pblock = (process_control_block_t*)(ADDR_8MB - process_count*ADDR_8KB);
	
	/* Check for valid parameters */
	if(buf == NULL)
	{
		return FAIL;
	}
	
	/* Check if the arguments are present */
	if(current_pblock->argument_length <= 1)
	{
		return FAIL;
	}
	
	if(current_pblock->argument_length > nbytes)
	{
		return FAIL;
	}
	
	/* Copy over the arguments */
	memcpy(buf, current_pblock->argument_buffer, nbytes);
	
	/* Succeed */
	return SUCCESS;
	
}

/*
* int32_t vidmap(uint8_t** screen_start)
* INPUTS: (screen_start) start of the screen
* OUTPUTS: 1 on success, -1 on failure
* DESCRIPTION: points to the proper video memory given the screen start
*/
int32_t vidmap(uint8_t** screen_start)
{
	/* Create test variable off of screen start */
	uint32_t screen_test;
	screen_test = (uint32_t)screen_start;
	
	/* Check Validity */
	if(screen_test < TOP_PAGE)
	{
		return FAIL;
	}
	
	if(screen_test >= BOTTOM_PAGE)
	{
		return FAIL;
	}
	
	if(screen_test == NULL)
	{
		return FAIL;
	}
	
	/* Point to the video memory */
	*screen_start = (uint8_t*)VID_MEM;
	return SUCCESS;
}


/* int32_t set_handler(int32_t signum, void* handler_address)
 * INPUTS: None
 * OUTPUTS: None
 * DESCRIPTION return -1
 */
int32_t set_handler(int32_t signum, void* handler_address)
{
	return FAIL;
}

/* int32_t sigreturn(void)
 * INPUTS: None
 * OUTPUTS: None
 * DESCRIPTION return -1
 */
int32_t sigreturn(void)
{
	return FAIL;
}

/* void init_fd(process_control_block_t pcb)
 * INPUT: 	pcb - set pcb values to null and zero
 * OUTPUT: 	none
 * DESCRIPTION: initialize pcb
 */
void init_fd(process_control_block_t pcb)
{
	int i;
	for(i = 0; i < 8; i++)
	{
		pcb.fde[i].fop_ptr = NULL;
		pcb.fde[i].inode = NULL;
		pcb.fde[i].file_pos = 0;
		pcb.fde[i].in_use = 0;
	}

}


/* void init_process_ct()
 * INPUT: none
 * OUTPUT: none
 * DESCRIPTION: initialize process_count to zero.
 */
void init_process_ct()
{
	process_count = 0;
}
