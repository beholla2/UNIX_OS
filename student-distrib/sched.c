
#include "sched.h"
#include "types.h"
#include "i8259.h"
#include "idt.h"


/* AW */
/* pq_init()
 * DESCRIPTION:		Initializes the process queue by setting struct attributes (front, rear, pcbs[])
 * INPUTS:			NONE
 * RETURN VALUE:	NONE
 */
void pq_init()
{
	/* initializing process queue */
	process_q.front = 1;	/* need to initialize to 1 b/c we'll decrmement this in the first execute */
	process_q.rear = 0;
	
	int i;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		process_q.pcbs[i] = NULL;
	}
}



/* AW */
/* pq_get_active_term()
 * DESCRIPTION:		Returns the terminal number of the currently actively running process
 * INPUTS:			NONE
 * RETURN VALUE:	The terminal number of the current process
 */ 
uint32_t pq_get_active_term()
{
	if(process_count > 0)
		return pq_peak(&process_q)->terminal_num;
	else
		return 0;
}



/* AW */
/* pq_enqueue(process_queue_t* process_q_ptr, uint32_t process_num)
 * INPUTS:			process_q_ptr:	Pointer to the global process queue struct
 *					process_num:	Number of the process to add to the rear of the queue
 *									Valid range must be checked by caller 1 <= process_num <= MAX_PROCESSES
 * RETURN VALUE:	-1 for failure, 0 for success
 * PURPOSE: 		Given a process number, adds this process to the process queue.  Specifically, adds
 *					a pointer to the process's PCB to the global process queue struct.
 */
int pq_enqueue(process_queue_t* process_q_ptr, uint32_t process_num)
{

	int old_rear;
	if(process_num > MAX_PROCESSES){
		return FAIL;
	}

	/* calculate address of PCB for this process */
	process_control_block_t* this_pcb = (process_control_block_t*)(ADDR_8MB - process_num*ADDR_8KB);

	/* update rear of queue */
	old_rear = process_q_ptr->rear;	/* save old rear to revert if error occurs */
	if( (process_q_ptr->rear) >= (MAX_PROCESSES - 1) ){
		process_q_ptr->rear = 0;		/* if we're at the end of array, move to start */
	} else{
		process_q_ptr->rear++;			/* else increment rear*/
	}

	/* check for overfilled queue */
/*	if(process_q_ptr->rear == process_q_ptr->front){
		process_q_ptr->rear = old_rear;
		return FAIL;
	}
*/	/* This doesn't work--we need to check using process_count or figure out a smarter way using front and rear */

	/* insert pcb pointer at rear of queue */
	process_q_ptr->pcbs[process_q_ptr->rear] = this_pcb;

	return SUCCESS;
}



/* AW */
/* pq_enqueue_front(process_queue_t* process_q_ptr, process_control_block_t* this_pcb)
 * DESCRIPTION:		This is a specialty function to be called within the execute system call.  When a new process
 *					starts running, it joins the queue from the front, not the rear.
 * INPUTS:			process_q_ptr:	Pointer to the global process queue struct
 *					process_num:	Number of the process to add to the front of the queue
 *									Valid range must be checked by caller 1 <= process_num <= MAX_PROCESSES
 * RETURN VALUE:	-1 for failure, 0 for success
 */
int pq_enqueue_front(process_queue_t* process_q_ptr, process_control_block_t* this_pcb)
{
	/* make space at front of queue */
	process_q_ptr->front--;

	if(process_q_ptr->front < 0){
		process_q_ptr->front = MAX_PROCESSES - 1;
	}

	process_q_ptr->pcbs[process_q_ptr->front] = this_pcb;

	return SUCCESS;
}



/* AW */
/* pq_peak(process_queue_t* process_q_ptr)
 * INPUTS:			process_q_ptr:	Pointer to the global process queue struct
 * RETURN VALUE:	A pointer to the process control block of the currently running process (the process at the front of the queue)
 * PURPOSE: 		Allows you to get a pointer to the pcb of the currently running process without altering the process queue.
 */ 
process_control_block_t* pq_peak(process_queue_t* process_q_ptr)
{
	/* return the pointer on the rear of the queue */
	return process_q_ptr->pcbs[process_q_ptr->front];
}


/* AW */
/* pq_dequeue(process_queue_t* process_q_ptr)
 * INPUTS:			process_q_ptr:	Pointer to the global process queue struct
 * RETURN VALUE:	A pointer to the process control block of the currently running process (the process at the front of the queue)
 * SIDE EFFECTS:	Removes the front process from the global process queue
 * PURPOSE: 		Removes the currently running (front) process from the global process queue and returns a pointer to its PCB.
 */ 
process_control_block_t* pq_dequeue(process_queue_t* process_q_ptr)
{

	/* set return value for dequeue */ 
	process_control_block_t * ret_block = process_q_ptr->pcbs[process_q_ptr->front];

	/* increment front index effectively moving next item to front of queue */
	process_q_ptr->front++;
	if(process_q_ptr->front >= MAX_PROCESSES)
	{
		process_q_ptr->front = 0;
	}

	return ret_block;
}


/* AW */
/* pq_rotate(process_queue_t* process_q_ptr)
 * INPUTS:			process_q_ptr:	Pointer to the global process queue struct
 * RETURN VALUE:	NONE
 * SIDE EFFECTS:	Updates the queue.  The next process moves to the front.  The front process moves to the rear.  
 * PURPOSE: 		Rotates the queue by moving the next-in-line process to the front and the front process to the rear.
 */ 
void pq_rotate(process_queue_t* process_q_ptr)
{
	/* dequeue the old front */
	process_control_block_t* old_front = pq_dequeue(process_q_ptr);

	/* and enqueue it at the rear */
	process_q_ptr->rear++;					/* increment rear - don't need to check bounds b/c we just dequeued */
	process_q_ptr->pcbs[process_q_ptr->rear] = old_front;	/* insert pcb pointer at rear */

}



/* change_task(register_t regs)
 * INPUT: 		regs - 
 * OUTPUTS: 	none
 * DESCRIPTION: From the process queue, process_q we want to context switch to 
 * 				the next process in the queue. This requires saving the current
 * 				processes registers
 */
void change_task(registers_t regs)
{
	/* create critical section */
	cli();

	/* get the current process from the top of the queue */
	process_control_block_t* curr_pcb = pq_peak(&process_q);

	/* Save all the registers in the current process pcb */
	save_pcb_regs(curr_pcb, regs);

	/* rotate queue so that we can get 'next' process */
	pq_rotate(&process_q);
	process_control_block_t* next_pcb = pq_peak(&process_q);


	int data_segment;
	uint32_t code_segment;
	uint32_t tmp_esp;

	//Test which space that we are returning in
	if(next_pcb->cs == KERNEL_CS) //defined in x86_desc.h
	{
		data_segment = KERNEL_DS;
		code_segment = KERNEL_CS;
		tmp_esp = next_pcb->esp;
	}
	else
	{
		data_segment = USER_DS;
		code_segment = USER_CS;
		tmp_esp = next_pcb->user_esp;
	}


	uint32_t tmp_eip = next_pcb->eip;
	//uint32_t tmp_ebp = next_pcb->ebp;
	uint32_t tmp_edi = next_pcb->edi;
	uint32_t tmp_esi = next_pcb->esi;
	uint32_t tmp_ebx = next_pcb->ebx;
	uint32_t tmp_edx = next_pcb->edx;
	uint32_t tmp_ecx = next_pcb->ecx;
	uint32_t tmp_eax = next_pcb->eax;
	uint32_t tmp_eflags = next_pcb->eflags; 
	tss.esp0 = ADDR_8MB - (next_pcb->pid)*ADDR_8KB - 4;	/* set address of kernel stack pointer */

	/* update memory space */
		pd_entry_t pde;		/* declare a pde struct */

		/* populate pde struct; first user prog is at 8MB, next at 12MB, etc */
		init_4mb_user_pde(&pde, ADDR_4MB + next_pcb->pid*ADDR_4MB);

		pd[PD_IDX_USER] = pde.val;	/* enter page directory entry into page directory */

		/* set cr3 */
		set_cr3(pd);	/* (flushes TLB) */

	send_eoi(0);	/* end of interrupt */

	//asm volatile("movl %0, %%eip" :: "g" (tmp_eip));
	//asm volatile("movl %0, %%ebp" :: "g" (tmp_ebp));
	asm volatile("movl %0, %%edi" :: "g" (tmp_edi));
	asm volatile("movl %0, %%esi" :: "g" (tmp_esi));
	asm volatile("movl %0, %%ebx" :: "g" (tmp_ebx));
	asm volatile("movl %0, %%edx" :: "g" (tmp_edx));
	asm volatile("movl %0, %%ecx" :: "g" (tmp_ecx));
	asm volatile("movl %0, %%eax" :: "g" (tmp_eax));
	asm volatile("pushl %0" :: "g" (data_segment));		/* push data segment of kernel or user */
	asm volatile("pushl %0" :: "g" (tmp_esp));			/* push stack pointer of the space we're switching to */
	asm volatile("pushl %0" :: "g" (tmp_eflags));		/* push flags */
	asm volatile("popl %%eax" :);
	asm volatile("orl $0x286, %%eax" :::"eax");
	asm volatile("pushl %%eax" :);
	asm volatile("pushl %0" :: "g" (code_segment));
	asm volatile("pushl %0" :: "g" (tmp_eip));			/* push destination eip */
	asm volatile("iret");
	
	 sti();
	/* end critical section */
}


/* save_pcb_regs(process_control_block_t* curr, registers_t regs)
 * INPUT: 		regs - register values to save into curr's registers
 *				curr - process_control_block pointer of the current process
 * OUTPUTS: 	none
 * DESCRIPTION: Saves curr's pcb with register values. 
 */
void save_pcb_regs(process_control_block_t* curr, registers_t regs)
{
	curr->eip = regs.eip;
	curr->esp = regs.esp;
	curr->ebp = regs.ebp;
	curr->edi = regs.edi;   
	curr->esi = regs.esi;
	curr->ebx = regs.ebx;
	curr->edx = regs.edx; 
	curr->ecx = regs.ecx;
	curr->eax = regs.eax;
	curr->cs =  regs.cs;
	curr->eflags =  regs.eflags;
	curr->user_esp = regs.useresp;
	curr->ss = regs.ss; //Segment Selector: Tells us if we are in userspace or kernel space
}




