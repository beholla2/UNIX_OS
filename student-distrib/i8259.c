/* *********************************************************
# FILE NAME: i8259.c
* PURPOSE: functions to initialize the pic
# AUTHOR: Queeblo OS
* MODIFIED: 10/27/2014
********************************************************* */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

/* void i8259_init(void)
 * INPUT: none
 * OUTPUT: none
 * DESCRIPTION: initializes the i8259 PIC
 */
void i8259_init(void)
{	
	/* ZR - once masked out, lets initialize master PIC */
	outb(ICW1, MASTER_8259_PORT); 			/* first control word - master  */
	outb(ICW2_MASTER, MASTER_8259_DATA); 	/* second control word - master */
	outb(ICW3_MASTER, MASTER_8259_DATA); 	/* third control word - master  */
	outb(ICW4, MASTER_8259_DATA);			/* fourth control word - master */

	/* ZR - time for the slave PIC */
	outb(ICW1, SLAVE_8259_PORT); 			/* first control word - slave  */
	outb(ICW2_SLAVE, SLAVE_8259_DATA); 		/* second control word - slave */
	outb(ICW3_SLAVE, SLAVE_8259_DATA); 		/* third control word - slave  */
	outb(ICW4, SLAVE_8259_DATA);			/* fourth control word - slave */
}

/* void enable_irq(uint32_t irq_num)
 * INPUT: irq_num - the irq line to enable
 * OUTPUT: none
 * DESCRIPTION: enables and unmasks the given irq line
 */
void enable_irq(uint32_t irq_num)
{
	if(irq_num >= IRQ_SLAVE) {
		/* ZR - opposite of disable, lets set these bits high */
		/* may want to check me on this one */
		slave_mask = inb(SLAVE_8259_DATA) & ~(1 << (irq_num - IRQ_SLAVE));
		outb(slave_mask, SLAVE_8259_DATA);
	}
	else {
		master_mask = inb(MASTER_8259_DATA) & ~(1 << irq_num);
		outb(master_mask, MASTER_8259_DATA);
	}

}

/* void disable_irq(uint32_t irq_num)
 * INPUT: irq_num - the irq line to disable
 * OUTPUT: none
 * DESCRIPTION: disables and masks the given irq line
 */
void disable_irq(uint32_t irq_num)
{
	if(irq_num >= IRQ_SLAVE) {	
		/* ZR - use slave */
		slave_mask = inb(SLAVE_8259_DATA) | (1 << (irq_num - IRQ_SLAVE)); /* grab what to mask */
		outb(slave_mask, SLAVE_8259_DATA); 	/* mask slave */
	}
	else {
		/* ZR - use master */
		master_mask = inb(MASTER_8259_DATA) | (1 << irq_num);
		outb(master_mask, MASTER_8259_DATA); 
	}
}

/* void send_eoi(uint32_t irq_num)
 * INPUT: irq_num - the irq line that signaled end of interrupt
 * OUTPUT: none
 * DESCRIPTION: writes the end of interrupt to the pic port
 */
void send_eoi(uint32_t irq_num)
{
	/* ZR - sending EOI */
	if(irq_num >= IRQ_SLAVE)
	{
		outb((EOI | (irq_num-IRQ_SLAVE)), SLAVE_8259_PORT);
		send_eoi(IRQ_CAS);
	}
	else
		outb((EOI | irq_num), MASTER_8259_PORT);
}

