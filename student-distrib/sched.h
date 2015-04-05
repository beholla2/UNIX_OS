#ifndef SCHED_H
#define SCHED_H

#include "syscalls.h"
#include "idt.h"
#include "i8259.h"

#define MAX_PROCESSES	6



/* AW */
/* Definition of the process_queue_t struct
 * This queue will be used by the scheduler to rotate b/t processes.
 */
typedef struct process_queue
{
	process_control_block_t* pcbs[MAX_PROCESSES];	/* We are implementing the queue as an array of pcb pointers */
	int front;										/* The index that is currently the front of the queue */
	int rear;										/* The index that is currently the rear of the queue */
													/* We would also include process_count, but that is already a global variable */
} process_queue_t;

extern process_queue_t process_q;



/* AW */
/* pq_get_active_term()
 * DESCRIPTION:		Returns the terminal number of the currently actively running process
 * INPUTS:			NONE
 * RETURN VALUE:	The terminal number of the current process
 */ 
uint32_t pq_get_active_term();



/* AW */
/* pq_init()
 * DESCRIPTION:		Initializes the process queue by setting struct attributes (front, rear, pcbs[])
 * INPUTS:			NONE
 * RETURN VALUE:	NONE
 */
void pq_init();



/* AW */
/* pq_enqueue(process_queue_t* process_q_ptr, uint32_t process_num)
 * INPUTS:			process_q_ptr:	Pointer to the global process queue struct
 *					process_num:	Number of the process to add to the rear of the queue
 *									Valid range must be checked by caller 1 <= process_num <= MAX_PROCESSES
 * RETURN VALUE:	-1 for failure, 0 for success
 * PURPOSE: 		Given a process number, adds this process to the process queue.  Specifically, adds
 *					a pointer to the process's PCB to the global process queue struct.
 */
int pq_enqueue(process_queue_t* process_q_ptr, uint32_t process_num);



/* AW */
/* pq_enqueue_front(process_queue_t* process_q_ptr, process_control_block_t* this_pcb)
 * DESCRIPTION:		This is a specialty function to be called within the execute system call.  When a new process
 *					starts running, it joins the queue from the front, not the rear.
 * INPUTS:			process_q_ptr:	Pointer to the global process queue struct
 *					process_num:	Number of the process to add to the front of the queue
 *									Valid range must be checked by caller 1 <= process_num <= MAX_PROCESSES
 * RETURN VALUE:	-1 for failure, 0 for success
 */
int pq_enqueue_front(process_queue_t* process_q_ptr, process_control_block_t* this_pcb);



/* AW */
/* pq_peak(process_queue_t* process_q_ptr)
 * INPUTS:			process_q_ptr:	Pointer to the global process queue struct
 * RETURN VALUE:	A pointer to the process control block of the currently running process (the process at the front of the queue)
 * PURPOSE: 		Allows you to get a pointer to the pcb of the currently running process without altering the process queue.
 */ 
process_control_block_t* pq_peak(process_queue_t* process_q_ptr);



/* AW */
/* pq_dequeue(process_queue_t* process_q_ptr)
 * INPUTS:			process_q_ptr:	Pointer to the global process queue struct
 * RETURN VALUE:	A pointer to the process control block of the currently running process (the process at the front of the queue)
 * SIDE EFFECTS:	Removes the front process from the global process queue
 * PURPOSE: 		Removes the currently running (front) process from the global process queue and returns a pointer to its PCB.
 */ 
process_control_block_t* pq_dequeue(process_queue_t* process_q_ptr);



/* AW */
/* pq_rotate(process_queue_t* process_q_ptr)
 * INPUTS:			process_q_ptr:	Pointer to the global process queue struct
 * RETURN VALUE:	NONE
 * SIDE EFFECTS:	Updates the queue.  The next process moves to the front.  The front process moves to the rear.  
 * PURPOSE: 		Rotates the queue by moving the next-in-line process to the front and the front process to the rear.
 */ 
void pq_rotate(process_queue_t* process_q_ptr);



/* 
 * switch_task(registers_t regs)
 * When the scheduler is called, it saves the stack and 
 * base pointers in a task structure, restores the stack and base 
 * pointers of the process to switch to, switches address spaces, and 
 * jumps to the instruction that the new task left off at the last time it was swapped.
 *
 */
void change_task(registers_t regs);


/* 
 * save_pcb_regs
 * 
 *
 */
void save_pcb_regs(process_control_block_t* curr, registers_t regs);


#endif /* SCHED_H */


