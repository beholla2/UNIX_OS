/* *********************************************************
# FILE NAME: page.c
* PURPOSE: functions to initialize paging
# AUTHOR: Queeblo OS
* MODIFIED: 12/7/2014
********************************************************* */

#define PAGE_ENTRY				1024
#define KERNEL_ENTRY			0x00400183
/* specific bit controls */
#define PRESENT					0x1
#define READWRITE				0x2
#define USER					0x4
#define CACHE					0x8
#define IGNORE					0x80
#define PT_IDX_VIDMEM			184			/* AW The middle 10 digits of video memory (0xB8000) imply a page table index of 184 */
#define PT_IDX_T1_VID			185
#define PT_IDX_T2_VID			186
#define PT_IDX_T3_VID			187
/* AW Page Table Entries - the high 20 bits are the page base address */
#define PTE_VIDMEM				0x000B8007	/* AW reminder: the 7's are in the low 12 bits so they are just control bits */
#define PTE_TERM1_VID			0x000B9007	/* AW incrementing from 0000 to 1000 is a difference of 4096 bytes (4 kB) */
#define PTE_TERM2_VID			0x000BA007
#define PTE_TERM3_VID			0x000BB007
#define BACKING_PAGES_START		0x000B9000	/* AW the address of the first video backing page; subsequent backing pages will
												follow at 4kB intervals */
#define CR4_PSE					0x00000010
#define CR0_VALUE				0x80000000

#include "page.h"
#include "x86_desc.h"
#include "lib.h"
#include "terminal.h"

/* void set_read_write()
 * INPUT: none
 * OUTPUT: none
 * DESCRIPTION: initializes the page directory by setting the read/write for all of the page tables
 */
void set_read_write()
{	
	int i;
	for(i = 0; i < PAGE_ENTRY; i++) {
		pd[i] = READWRITE;				/* set bit 1 */
	}
}

/* void init_table()
 * INPUT: none
 * OUTPUT: none
 * DESCRIPTION: initializes page tables
 */
void init_table()
{
	int i;
	for(i = 0; i < PAGE_ENTRY; i++) {
		pt_0_4[i] = READWRITE;
	}
	/* video memory */
	pt_0_4[PT_IDX_VIDMEM] = PTE_VIDMEM;
	/* video backing pages for terminals */
	pt_0_4[PT_IDX_T1_VID] = PTE_TERM1_VID;
	pt_0_4[PT_IDX_T2_VID] = PTE_TERM2_VID;
	pt_0_4[PT_IDX_T3_VID] = PTE_TERM3_VID;	

}

/* void init_page()
 * INPUT: none
 * OUTPUT: none
 * DESCRIPTION: initializes the kernel page and video memory,
 * 				kernel and Video memory set up in paging scheme, CR3 and CR4 set to appropriate values.
 */
void init_page()
{
	set_read_write();
	init_table();
	
	int PDE_video = 0;
	int PDE_kernel = 1;
	
	uint32_t reg_cr0 = 0;
	uint32_t reg_cr4 = 0;
	
	pd[PDE_video] = (uint32_t) pt_0_4 | USER | PRESENT | READWRITE;
	pd[PDE_kernel] = KERNEL_ENTRY;
	/* sets c variable reg_cr4 equal to register cr4 */
	asm volatile ("mov %%CR4, %0;"
					: "=c"(reg_cr4));	
	reg_cr4 |= CR4_PSE;
	/* put modified value back into register cr4 */
	asm volatile ("mov %0, %%CR4;"
					:
					: "c"(reg_cr4));	
	/* puts address of page directory into cr3 */
	asm volatile ("mov %0, %%CR3"
					:
					: "c"(pd));		
	/* sets c variable reg_cr0 equal to register cr0 */
	asm volatile ("mov %%CR0, %0"
					: "=c"(reg_cr0));	
	reg_cr0 |= CR0_VALUE;
	/* put modified value back into register cr0 */
	asm volatile ("mov %0, %%CR0"
					:
					: "c"(reg_cr0));
}

/* void init_4mb_user_pde(pd_entry_t* pde, uint32_t page_base_addr)
 * INPUT: pd_entry_t* pde - page directory entry to initialize
 		  uint32_t page_base_addr - used to get pointer to 4MB aligned page
 * OUTPUT: none
 * DESCRIPTION: initializes a 4mb user page directory entry
 */
void init_4mb_user_pde(pd_entry_t* pde, uint32_t page_base_addr)
{
	pde->is_present = 1;
	pde->rw = 1;				/* read and write */
	pde->user_supervisor = 1;	/* user level privelege */
	pde->write_through = 1;		/* no clue...1 means write-through caching, 0 means write-back caching. */
	pde->cache_disabled = 0;	/* 0 means page or page table can be cached */
	pde->is_accessed = 0;		/* initially not accessed */
	pde->is_dirty = 0;			/* initially clean */
	pde->is_big_page = 1;		/* 1 means 4 MB page */
	pde->is_global_page = 0;	/* not a global page */
	pde->user_reserved  = 0;	/* Bits 9, 10, and 11 are available for use by software */
	pde->pat = 0;				/* page attribute table idx unused */
	pde->reserved = 0;			/* Bits 13 thru 21  (9 bits) */
	pde->page_base_addr = (page_base_addr >> 22);	/* pointer to 4MB aligned page */
}

/* void set_cr3(uint32_t* page_dir)
 * INPUT: uint32_t* page_dir - process' page directory
 * OUTPUT: none
 * DESCRIPTION: set cr3 register to point to a give process' page directory 
 */
void set_cr3(uint32_t* page_dir)
{
	/* puts address of page directory into cr3 */
	asm volatile ("mov %0, %%CR3"
		:
		: "c"(page_dir));
}

/* uint8_t* get_backing_page(int terminal_num)
 * INPUT: int terminal_num - the number of the terminal to get the backing page of
 * OUTPUT: pointer to the backing page or -1 if invalid terminal number
 * DESCRIPTION: given a terminal number, returns a pointer to the video backing page of that terminal
 *				NOTE: Assumes terminal_num is valid; valid range is 0 to NUM_TERMINALS - 1
 */
uint8_t* get_backing_page(int terminal_num)
{
	return (uint8_t*)(BACKING_PAGES_START + terminal_num * BYTES_4KB);
}

/* int copy_4kb_page(uint8_t* source, uint8_t* dest)
 * INPUT: uint8_t* source - virtual address of the source page		
 *		  uint8_t* dest - virtual address of the destination page			
 * OUTPUT:	0 for success, -1 for failure
 * DESCRIPTION: given a pointer to two 4kB pages, this function copies one page to the other
 *				NOTE: As of now, this may only work if the two pages are in the same page table.
 */
int copy_4kb_page(uint8_t* source, uint8_t* dest)
{
	memcpy(dest, source, BYTES_4KB);

	return 0;
}
