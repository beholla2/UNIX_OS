/* *********************************************************
# FILE NAME: page.h
* PURPOSE: header for page.c
# AUTHOR: Queeblo OS
* MODIFIED: 12/07/2014
********************************************************* */

#ifndef _PAGE_H
#define _PAGE_H

#define NUM_PD_ENTRIES			1024
#define BYTES_4KB				4096

#include "types.h"

typedef struct pd_entry_t {
	union {
		uint32_t val;
		struct{
			uint32_t is_present : 1;
			uint32_t rw : 1;
			uint32_t user_supervisor : 1;
			uint32_t write_through : 1;
			uint32_t cache_disabled : 1;
			uint32_t is_accessed : 1;
			uint32_t is_dirty : 1;
			uint32_t is_big_page : 1;
			uint32_t is_global_page : 1;

			uint32_t user_reserved : 3;
			uint32_t pat : 1;
			uint32_t reserved : 9;
			uint32_t page_base_addr : 10;
		} __attribute__((packed));
	};
} pd_entry_t;

struct page_directory {
	uint32_t entries[NUM_PD_ENTRIES];
} __attribute__ ((aligned (BYTES_4KB)));
typedef struct page_directory pd_t;

/* starts paging in kernel.c */
void init_page();
/* populates entries for system calls */
void init_4mb_user_pde(pd_entry_t * pde, uint32_t page_base_addr);
/* flush TLB for system calls */
void set_cr3(uint32_t* page_dir);

/* used for saving terminal state and switching terminals */
uint8_t* get_backing_page(int terminal_num);
int copy_4kb_page(uint8_t* source, uint8_t* dest);

#endif	/* _PAGE_H */
