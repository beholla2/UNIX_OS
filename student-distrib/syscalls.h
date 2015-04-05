/*	*********************************************************
	# FILE NAME: syscalls.h
	# PURPOSE: header for syscalls.c
	# AUTHOR: Queeblo OS
	# CREATED: 11/12/2014
	# MODIFIED: 11/17/2014
	********************************************************* */
#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include "types.h"
#include "page.h"
#include "filesys_mod.h"
#include "lib.h"
#include "x86_desc.h"
#include "rtc.h"
#include "terminal.h"
#include "keyboard.h"




#define ASCII_SPACE 	0x20
#define PD_IDX_USER		0x20		/* virtual address 128 MB makes high 10 bits equal 00 0010 0000*/
#define EXEC_CHAR		0x7F
#define ADDR_4MB		0x00400000
#define ADDR_8MB		0x00800000
#define ADDR_128MB		0x8000000
#define ADDR_8KB 		0x2000
#define PROG_IMG_OFFSET	0x48000
#define ELF_STR			0x7F454C46
#define LOW8_BITMASK	0x000000FF
#define OPS_SIZE		8
#define MAX_PROCESS		9
#define USE				0x1
#define SUCCESS			0
#define FAIL			-1
#define INDEX			1
#define ARG_BUFF_SIZE	128
#define TOP_PAGE		0x08000000
#define BOTTOM_PAGE		0x08400000
#define VID_MEM 		0x000B8000
#define PD_IDX_VID		0x40

	
typedef int32_t(*fops_open_t)(void);
typedef int32_t(*fops_read_t)(int32_t, void*, int32_t);
typedef int32_t(*fops_write_t)(int32_t, const void*, int32_t);
typedef int32_t(*fops_close_t)(void);

typedef struct fops_functions {
	fops_read_t		function_read;
	fops_write_t	function_write;
	fops_open_t		function_open;
	fops_close_t	function_close;
}  fops_functions_t;

typedef struct fd_entry_t {
	fops_functions_t * fop_ptr;
	inode_t * inode;
	uint8_t file_name[FNAME_LENGTH];	/* AW added as a hack to try to get read_file to work */
	uint32_t file_pos;
	uint32_t in_use;
} fd_entry_t;

typedef struct process_control_block_t {

	/* registers */ 
	uint32_t eip;							/* The instruction pointer to iret to */
	uint32_t esp;							/* The stack pointer of the process--we may need a separate one for
											 * kernel and user space? */
	uint32_t ebp;							/* The base pointer--again we may need separate user and kernel ebps */
	uint32_t edi;   						/* general purpose register */
	uint32_t esi;   						/* general purpose register */
	uint32_t ebx;   						/* general purpose register */
	uint32_t edx;   						/* general purpose register */
	uint32_t ecx;   						/* general purpose register */
	uint32_t eax;   						/* general purpose register */
	uint32_t cs;
	uint32_t eflags;
	uint32_t user_esp;						/* esp in user space */
	uint32_t ss;
	uint32_t pid;							/* This process's process number */
	uint32_t parent_ptr;					/* Pointer to parent process's PCB */
	uint32_t terminal_num;					/* The terminal this process is running in */
	fd_entry_t fde[OPS_SIZE]; 				/* File descriptor array */
	uint32_t argument_length;				/* Length (in bytes) of the argument passed to this process */
	uint8_t argument_buffer[ARG_BUFF_SIZE];	/* Buffer containing the argument passed to this process */

} process_control_block_t;


uint32_t check_use (int32_t fd);
void systemcalls_initialize (void);

extern uint32_t process_count;

extern int primary_shell_count;


int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command_arg);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytyes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd, const void* buf, int32_t nbytyes);
int32_t getargs(uint8_t* buf, int32_t nbytyes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);
void init_fd(process_control_block_t pcb);
void args_initialize(const uint8_t * command, uint8_t * char_space_indices, int32_t num_spaces, process_control_block_t * current_pblock);

#endif /* _SYSCALLS_H */


