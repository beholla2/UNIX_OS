/* *********************************************************
# FILE NAME: idt.c
* PURPOSE: functions to populate idt, int and irq handlers
# AUTHOR: Queeblo OS
* MODIFIED: 10/27/2014
**********************************************************/

#include "idt.h"
#include "x86_desc.h"
#include "lib.h"
#include "handler.h"
#include "rtc.h"
#include "i8259.h"
#include "keyboard.h"
#include "syscalls.h"
#include "sched.h"



/* index is int num, value is description */
static const char *int_desc[] = {
	"divide by zero", "debug", "non maskable interrupt", "breakpoint", "overflow",
	"bounds check", "invalid opcode", "coprocessor not available", "double fault", 
	"reserved", "ece391invalid TSS", "segment not present", "stack exception", "general protection fault",
	"page fault", "reserved", "coprocessor error", "alignment error"
};

/* local functions */
void idt_handler(registers_t regs);
void idt_set_vector(uint8_t vec, uint32_t dpl, void* handler_address);
void rtc_handler();
void pit_handler(registers_t regs);

/* idt_init()
 * INPUT: none
 * OUTPUT: none
 * DESCRIPTION: populates entries in the idt table using helper
 * 				function idt_set_vector
 */
void idt_init()
{
	idt_set_vector(0, KERNEL_DPL, &handler_0);
	idt_set_vector(1, KERNEL_DPL, &handler_1);
	idt_set_vector(2, KERNEL_DPL, &handler_2);
	idt_set_vector(3, KERNEL_DPL, &handler_3);
	idt_set_vector(4, USER_DPL, &handler_4);
	idt_set_vector(5, USER_DPL, &handler_5);
	idt_set_vector(6, KERNEL_DPL, &handler_6);
	idt_set_vector(7, KERNEL_DPL, &handler_7);
	idt_set_vector(8, KERNEL_DPL, &handler_8);
	idt_set_vector(9, KERNEL_DPL, &handler_9);
	idt_set_vector(10, KERNEL_DPL, &handler_10);
	idt_set_vector(11, KERNEL_DPL, &handler_11);
	idt_set_vector(12, KERNEL_DPL, &handler_12);
	idt_set_vector(13, KERNEL_DPL, &handler_13);
	idt_set_vector(14, KERNEL_DPL, &handler_14);
	idt_set_vector(15, KERNEL_DPL, &handler_15);
	idt_set_vector(16, KERNEL_DPL, &handler_16);
	idt_set_vector(17, KERNEL_DPL, &handler_17);
	idt_set_vector(18, KERNEL_DPL, &handler_18);
	idt_set_vector(19, KERNEL_DPL, &handler_19);
	idt_set_vector(20, KERNEL_DPL, &handler_20);
	idt_set_vector(21, KERNEL_DPL, &handler_21);
	idt_set_vector(22, KERNEL_DPL, &handler_22);
	idt_set_vector(23, KERNEL_DPL, &handler_23);
	idt_set_vector(24, KERNEL_DPL, &handler_24);
	idt_set_vector(25, KERNEL_DPL, &handler_25);
	idt_set_vector(26, KERNEL_DPL, &handler_26);
	idt_set_vector(27, KERNEL_DPL, &handler_27);
	idt_set_vector(28, KERNEL_DPL, &handler_28);
	idt_set_vector(29, KERNEL_DPL, &handler_29);
	idt_set_vector(30, KERNEL_DPL, &handler_30);
	idt_set_vector(31, KERNEL_DPL, &handler_31);

	idt_set_vector(32, KERNEL_DPL, &handler_irq0);
	idt_set_vector(33, KERNEL_DPL, &handler_irq1);
	idt_set_vector(34, KERNEL_DPL, &handler_irq2);
	idt_set_vector(35, KERNEL_DPL, &handler_irq3);
	idt_set_vector(36, KERNEL_DPL, &handler_irq4);
	idt_set_vector(38, KERNEL_DPL, &handler_irq6);
	idt_set_vector(40, KERNEL_DPL, &handler_irq8);
	idt_set_vector(42, KERNEL_DPL, &handler_irq10);
	idt_set_vector(43, KERNEL_DPL, &handler_irq11);
	idt_set_vector(44, KERNEL_DPL, &handler_irq12);
	idt_set_vector(45, KERNEL_DPL, &handler_irq13);
	idt_set_vector(46, KERNEL_DPL, &handler_irq14);
	idt_set_vector(47, KERNEL_DPL, &handler_irq15);

	idt_set_vector(SYSCALL, USER_DPL, &handler_syscall);

	lidt(idt_desc_ptr);
}

/* idt_set_vector(uint8_t vec, uint32_t dpl, void* handler_address)
 * INPUT: vec - interrupt number
 *		  dpl - privilege level
 *	      handler_address - address to handler, handlers are in handler.S
 * OUTPUT: none
 * DESCRIPTION: helper function to initialize an idt entry
 */
void idt_set_vector(uint8_t vec, uint32_t dpl, void* handler_address)
{
	SET_IDT_ENTRY(idt[vec], handler_address);

	idt[vec].seg_selector = KERNEL_CS;
	idt[vec].size = 1;
	idt[vec].dpl = dpl;
	idt[vec].present = 1;
	
	idt[vec].reserved4 = 0;
	idt[vec].reserved3 = 1;
	if (vec == SYSCALL) {
		idt[vec].reserved3 = 0;
	}
	idt[vec].reserved2 = 1;
	idt[vec].reserved1 = 1;
	idt[vec].reserved0 = 0;
}

/* idt_handler(registers_t regs)
 * INPUT: regs - contains register values, flags, pushed error code, and the interrupt number
 * OUTPUT: none
 * DESCRIPTION: prints the error code and the interrupt number to the screen
 * 				spins for fatal errors to indicate that the kernel should restart
 */
void idt_handler(registers_t regs)
{
	uint32_t flags;
	cli_and_save(flags);

	//clear();
	if (regs.int_num < SUPPORTED_INT) {
		printf("Error Code: %d\n", regs.error_code);
		printf("Int Num %d: %s\n", regs.int_num, int_desc[regs.int_num]);
	}
	else {
			printf("Int Num: %d", regs.int_num);
	}

	/* spin nicely */
	if(regs.int_num != 3 && regs.int_num !=4){
		asm volatile(".1: hlt; jmp .1;");
	}

	restore_flags(flags);
}

/* pic_handler(registers_t regs)
 * INPUT: regs - contains register values, flags, pushed error code, and the irq number
 * OUTPUT: none
 * DESCRIPTION: calls the appropriate handler associated with the irq number, then sends eoi signal
 */
void pic_handler(registers_t regs)
{
	uint32_t flags;
	cli_and_save(flags);     

	switch (regs.int_num) {
		case IRQ_0:
			pit_handler(regs);
			break;
		case IRQ_1:
			keyboard_handler();
			break;
		case IRQ_8:
			rtc_handler();
			break;
		default:
			break; 
	}
	send_eoi(regs.int_num);
	
	restore_flags(flags);
}

/* rtc_handler()
 * INPUT: none
 * OUTPUT: none
 * DESCRIPTION: allows future interrupts by reading rtc interrupt info and clearing old data
 */
void rtc_handler()
{
	clear_rtc_read();			/* break rtc_read */
	outb(REG_C, RTC_INDEX);
	inb(RTC_DATA);
}



/* pit_handler(registers_t regs)
 * INPUT: 	regs - register values used to populate next processes' context
 * OUTPUT: 	none
 * DESCRIPTION: calls our switch context helper function
 */
void pit_handler(registers_t regs)
{
	/* call our switching function 
	 * scheduling not working
	 */ 

	//if(process_count >= NUM_TERMINALS)
	//	change_task(regs);
}

